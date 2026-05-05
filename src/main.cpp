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
#include "dpdk_context.hpp"
#include "ingestor.hpp"

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
    constexpr uint16_t port_id = 0;
    DPDKContext dpdk_context(port_id);

    dpdk_context.setup_eal(argc, argv);
    dpdk_context.setup_mempool();
    dpdk_context.setup_eth_device(port_id);

    std::string outdir;

    if (argc != 2) {
        std::cout << "Please specify the file to parse and an output directory" << '\n';
        return 1;
    }

    outdir = argv[1];

    ITCH::ItchParser parser;
    BenchmarkOrderBook ob_bm_handler;
    BenchmarkParsing parsing_bm_handler;

    rte_mbuf* bufs[64];

    rte_eth_stats stats{};
    uint64_t last_print = rte_get_timer_cycles();
    uint64_t hz = rte_get_timer_hz();
    size_t total_size = 0;
    uint64_t msgs = 0;
    uint64_t pkts = 0;


    ITCH::Ingestor<BenchmarkOrderBook> ingestor(ob_bm_handler, dpdk_context);
    ingestor.ingest_messages();

    //while (!ob_bm_handler.should_stop()) {
    //    uint16_t n = rte_eth_rx_burst(port_id, 0, bufs, 64);
    //    pkts += n;

    //    for (int i = 0; i < n; ++i) {
    //        rte_mbuf* m = bufs[i];
    //        static int pkt_i = 0;

    //        if (m->nb_segs != 1) {
    //            printf("nonlinear: nb_segs=%u data_len=%u pkt_len=%u\n",
    //                   m->nb_segs, m->data_len, m->pkt_len);
    //        }

    //        std::byte* p = rte_pktmbuf_mtod(m, std::byte*);
    //        uint16_t len = m->pkt_len;
    //        p += sizeof(rte_ether_hdr);
    //        p += sizeof(rte_ipv4_hdr);
    //        auto* udp = reinterpret_cast<rte_udp_hdr*>(p);
    //        p += sizeof(rte_udp_hdr);

    //        p += 10; // temporary, MoldUDP64 of size 20 bytes
    //        uint64_t seq;
    //        std::memcpy(&seq, p, 8);
    //        seq = rte_be_to_cpu_64(seq);
    //        p += 8;

    //        uint16_t msg_count;
    //        std::memcpy(&msg_count, p, 2);
    //        msg_count = rte_be_to_cpu_16(msg_count);
    //        p += 2;

    //        msgs += msg_count;
    //        size_t itch_len = rte_be_to_cpu_16(udp->dgram_len) - sizeof(rte_udp_hdr) - 20;

    //        parser.parse(p, itch_len, ob_bm_handler);
    //        total_size += itch_len;
    //    }

    //    rte_pktmbuf_free_bulk(bufs, n);
    //    uint64_t now = rte_get_timer_cycles();
    //    if (now - last_print > hz) {
    //        std::cout << "Received ITCH: " << total_size << '\n';
    //        std::cout << "PpS: " << pkts << '\n';
    //        std::cout << "Msg/s: " << msgs << '\n';
    //        pkts = 0;
    //        msgs = 0;
    //        last_print = now;
    //    }
    //}

    #ifndef PERF
    export_latency_distribution_csv(ob_bm_handler, outdir + "parsing_and_order_book_latency_distribution.csv");
    export_prices_csv(ob_bm_handler.prices, outdir);
    #endif

    run_noise = false;

    return 0;
}
