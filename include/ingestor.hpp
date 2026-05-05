#pragma once
#include "itch_parser.hpp"
#include "dpdk_context.hpp"

#include <rte_ether.h>
#include <rte_ip4.h>
#include <rte_udp.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <iostream>

namespace ITCH {

template<typename Handler>
class Ingestor {
public:
    explicit Ingestor(Handler& handler, DPDKContext& dpdk_context)
        : handler_(handler), dpdk_context_(dpdk_context) {}

    void ingest_messages();

private:
    ItchParser parser_;
    Handler& handler_;
    DPDKContext& dpdk_context_;
};

[[gnu::noinline]] static void log_progress(size_t total_size, uint64_t pkts, uint64_t msgs) {
    std::cout << "Received ITCH: " << total_size << '\n';
    std::cout << "PpS: " << pkts << '\n';
    std::cout << "Msg/s: " << msgs << '\n';
}

template<typename Handler>
void Ingestor<Handler>::ingest_messages() {
    rte_mbuf* bufs[64];

    rte_eth_stats stats{};
    uint64_t last_print = rte_get_timer_cycles();
    uint64_t hz = rte_get_timer_hz();
    size_t total_size = 0;
    uint64_t msgs = 0;
    uint64_t pkts = 0;

    while (!handler_.should_stop()) {
        uint16_t n = rte_eth_rx_burst(dpdk_context_.get_port_id(), 0, bufs, 64);
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

            parser_.parse(p, itch_len, handler_);
            total_size += itch_len;
        }

        rte_pktmbuf_free_bulk(bufs, n);
        uint64_t now = rte_get_timer_cycles();
        if (now - last_print > hz) {
            log_progress(total_size, pkts, msgs);
            pkts = 0;
            msgs = 0;
            last_print = now;
        }
    }
}
}
