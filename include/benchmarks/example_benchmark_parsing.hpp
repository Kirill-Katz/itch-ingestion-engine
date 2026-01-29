#pragma once

#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include <emmintrin.h>
#include <benchmark/benchmark.h>
#include <x86intrin.h>
#include <iostream>
#include "itch_parser.hpp"

template<typename T>
inline void consume(T msg) {
    benchmark::DoNotOptimize(msg);
    benchmark::ClobberMemory();
}

struct BenchmarkParsing {
    inline void handle(ITCH::AddOrderNoMpid msg);
    inline void handle(ITCH::OrderCancel msg);
    inline void handle(ITCH::OrderDelete msg);
    inline void handle(ITCH::OrderReplace msg);

    inline void handle(ITCH::SystemEvent msg);
    inline void handle(ITCH::StockDirectory msg);
    inline void handle(ITCH::TradingAction msg);
    inline void handle(ITCH::RegSho msg);
    inline void handle(ITCH::MarketParticipantPos msg);
    inline void handle(ITCH::MwcbDeclineLevel msg);
    inline void handle(ITCH::MwcbStatus msg);
    inline void handle(ITCH::IpoQuotationPeriodUpd msg);
    inline void handle(ITCH::LuldAuctionCollar msg);
    inline void handle(ITCH::OperationalHalt msg);

    inline void handle(ITCH::AddOrderMpid msg);
    inline void handle(ITCH::OrderExecuted msg);
    inline void handle(ITCH::OrderExecutedPrice msg);

    inline void handle(ITCH::TradeMessageNonCross msg);
    inline void handle(ITCH::TradeMessageCross msg);
    inline void handle(ITCH::BrokenTrade msg);
    inline void handle(ITCH::Noii msg);
    inline void handle(ITCH::DirectListingCapitalRaise msg);

    void handle_after();
    void handle_before();
    void reset();

    absl::flat_hash_map<uint64_t, uint64_t> latency_distribution;
    uint64_t total_messages = 0;
    unsigned aux_start, aux_end;

    uint64_t t0;
};

inline void BenchmarkParsing::handle_before() {
    std::atomic_signal_fence(std::memory_order_seq_cst);
    t0 = __rdtscp(&aux_start);
}

inline void BenchmarkParsing::handle_after() {
    _mm_lfence();
    uint64_t t1 = __rdtscp(&aux_end);
    std::atomic_signal_fence(std::memory_order_seq_cst);

    auto cycles = t1 - t0;
    latency_distribution[cycles]++;
    total_messages++;
}

#define DEF_HANDLER(T) \
inline void BenchmarkParsing::handle(T msg) { \
    consume(msg); \
}

DEF_HANDLER(ITCH::AddOrderNoMpid)
DEF_HANDLER(ITCH::OrderCancel)
DEF_HANDLER(ITCH::OrderDelete)
DEF_HANDLER(ITCH::OrderReplace)
DEF_HANDLER(ITCH::SystemEvent)
DEF_HANDLER(ITCH::StockDirectory)
DEF_HANDLER(ITCH::TradingAction)
DEF_HANDLER(ITCH::RegSho)
DEF_HANDLER(ITCH::MarketParticipantPos)
DEF_HANDLER(ITCH::MwcbDeclineLevel)
DEF_HANDLER(ITCH::MwcbStatus)
DEF_HANDLER(ITCH::IpoQuotationPeriodUpd)
DEF_HANDLER(ITCH::LuldAuctionCollar)
DEF_HANDLER(ITCH::OperationalHalt)
DEF_HANDLER(ITCH::AddOrderMpid)
DEF_HANDLER(ITCH::OrderExecuted)
DEF_HANDLER(ITCH::OrderExecutedPrice)
DEF_HANDLER(ITCH::TradeMessageNonCross)
DEF_HANDLER(ITCH::TradeMessageCross)
DEF_HANDLER(ITCH::BrokenTrade)
DEF_HANDLER(ITCH::Noii)
DEF_HANDLER(ITCH::DirectListingCapitalRaise)

#undef DEF_HANDLER
