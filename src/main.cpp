#include <iostream>
#include <rte_ether.h>
#include <rte_ip4.h>
#include <rte_udp.h>
#include <vector>
#include <iostream>
#include <time.h>
#include <rte_eal.h>
#include <rte_ethdev.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>
#include <random>

#include "itch_parser.hpp"
#include "benchmarks/benchmark_utils.hpp"
#include "benchmarks/example_benchmark.hpp"
#include "benchmarks/example_benchmark_parsing.hpp"

std::atomic<bool> run_noise = true;

void allocator_noise() {
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> size_dist(4096, 8*4096);
    std::uniform_int_distribution<int> action(0, 1);

    std::vector<void*> blocks;

    while (run_noise.load()) {
        if (action(rng) == 0 || blocks.empty()) {
            size_t sz = size_dist(rng);
            void* p = std::malloc(sz);
            memset(p, 0xAA, sz);
            blocks.push_back(p);
        } else {
            size_t i = rng() % blocks.size();
            std::free(blocks[i]);
            blocks.erase(blocks.begin() + i);
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
    }
}


int main(int argc, char** argv) {
    int eal_argc = rte_eal_init(argc, argv);
    if (eal_argc < 0) {
        throw std::runtime_error("EAL init failed");
    }

    if (rte_eth_dev_count_avail() == 0) {
        throw std::runtime_error("Specify a vdev device");
    }

    uint16_t port_id = 0;
    rte_mempool* pool = rte_pktmbuf_pool_create(
        "mbuf_pool",
        1024,
        256,
        0,
        RTE_PKTMBUF_HEADROOM + 2048,
        rte_socket_id()
    );

    if (!pool) {
        throw std::runtime_error("mempool creation failed\n");
    }

    rte_eth_dev_info dev_info;
    if (rte_eth_dev_info_get(port_id, &dev_info) != 0)
        throw std::runtime_error("dev info failed");

    rte_eth_conf conf{};
    conf.txmode.offloads = 0;
    conf.rxmode.offloads = 0;

    if (rte_eth_dev_configure(port_id, 1, 1, &conf) < 0)
        throw std::runtime_error("dev configure failed");

    rte_eth_txconf txconf = dev_info.default_txconf;
    txconf.offloads = 0;

    rte_eth_rxconf rxconf = dev_info.default_rxconf;
    rxconf.offloads = 0;

    if (rte_eth_tx_queue_setup(port_id, 0, 1024,
                               rte_socket_id(), &txconf) != 0)
        throw std::runtime_error("tx queue failed");

    if (rte_eth_rx_queue_setup(port_id, 0, 1024,
                               rte_socket_id(), &rxconf, pool) != 0)
        throw std::runtime_error("rx queue failed");

    if (rte_eth_dev_start(port_id) < 0)
        throw std::runtime_error("dev start failed");

    argc -= eal_argc;
    argv += eal_argc;

    std::string filepath;
    std::string outdir;

    if (argc != 3) {
        std::cout << "Please specify the file to parse and an output directory" << '\n';
        return 1;
    }

    filepath = argv[1];
    outdir   = argv[2];

    auto [src_buf, bytes_read] = init_benchmark(filepath);
    const std::byte* src = src_buf.data();
    size_t len = bytes_read;

    //std::thread noise(allocator_noise);

    ITCH::ItchParser parser;
    BenchmarkOrderBook ob_bm_handler;
    BenchmarkParsing parsing_bm_handler;

    rte_mbuf* bufs[64];

    std::ofstream out("../data/itch_out",
                  std::ios::binary | std::ios::out | std::ios::trunc);
    std::vector<char> buf;
    buf.reserve(1<<20);

    rte_eth_stats stats{};
    uint64_t last_print = rte_get_timer_cycles();
    uint64_t hz = rte_get_timer_hz();
    size_t total_size = 0;
    uint64_t msgs = 0;
    uint64_t pkts = 0;

    while (!ob_bm_handler.last_message) {
        uint16_t n = rte_eth_rx_burst(port_id, 0, bufs, 64);
        pkts += n;

        for (int i = 0; i < n; ++i) {
            rte_mbuf* m = bufs[i];
            static int pkt_i = 0;

            if (m->nb_segs != 1) {
                printf("nonlinear: nb_segs=%u data_len=%u pkt_len=%u\n",
                       m->nb_segs, m->data_len, m->pkt_len);
            }

            std::byte* p = rte_pktmbuf_mtod(m, std::byte*);
            uint16_t len = m->pkt_len;
            p += sizeof(rte_ether_hdr);
            p += sizeof(rte_ipv4_hdr);
            auto* udp = reinterpret_cast<rte_udp_hdr*>(p);
            p += sizeof(rte_udp_hdr);

            p += 10; // temporary, MoldUDP64 of size 20 bytes
            uint64_t seq;
            std::memcpy(&seq, p, 8);
            seq = rte_be_to_cpu_64(seq);
            p += 8;

            uint16_t msg_count;
            std::memcpy(&msg_count, p, 2);
            msg_count = rte_be_to_cpu_16(msg_count);
            p += 2;

            msgs += msg_count;
            size_t itch_len = rte_be_to_cpu_16(udp->dgram_len) - sizeof(rte_udp_hdr) - 20;

            if (rte_be_to_cpu_16(udp->dgram_len) + sizeof(rte_ipv4_hdr) + sizeof(rte_ether_hdr) != m->pkt_len) {
                std::cout << rte_be_to_cpu_16(udp->dgram_len) + sizeof(rte_ipv4_hdr) + sizeof(rte_ether_hdr) << ' ' << m->pkt_len << '\n';
                throw std::runtime_error("Something went wrong, pkt length doesn't match expected length");
            }

            parser.parse(p, itch_len, ob_bm_handler);
            total_size += itch_len;
        }

        rte_pktmbuf_free_bulk(bufs, n);
        uint64_t now = rte_get_timer_cycles();
        if (now - last_print > hz) {
            std::cout << "Received ITCH: " << total_size << '\n';
            std::cout << "PpS: " << pkts << '\n';
            std::cout << "Msg/s: " << msgs << '\n';
            pkts = 0;
            msgs = 0;
            last_print = now;
        }
    }

    #ifndef PERF
    //export_latency_distribution_csv(ob_bm_handler, outdir + "parsing_latency_distribution.csv");
    export_latency_distribution_csv(ob_bm_handler, outdir + "parsing_and_order_book_latency_distribution.csv");
    export_prices_csv(ob_bm_handler.prices, outdir);
    #endif

    run_noise = false;
    //noise.join();

    return 0;
}
