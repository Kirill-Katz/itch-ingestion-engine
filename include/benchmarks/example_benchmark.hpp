#pragma once

#include <benchmark/benchmark.h>
#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include <emmintrin.h>
#include <x86intrin.h>
#include "itch_parser.hpp"
#include "levels/vector_levels_b_search_split.hpp"
#include "order_book.hpp"
#include "order_book_shared.hpp"

struct BenchmarkOrderBook {
    uint16_t target_stock_locate = -1;

    void handle(const ITCH::StockDirectory&);
    void handle(const ITCH::AddOrderNoMpid&);
    void handle(const ITCH::AddOrderMpid&);
    void handle(const ITCH::OrderExecuted&);
    void handle(const ITCH::OrderExecutedPrice&);
    void handle(const ITCH::OrderCancel&);
    void handle(const ITCH::OrderDelete&);
    void handle(const ITCH::OrderReplace&);
    void handle(const ITCH::SystemEvent&);

    void handle_change(const OB::BestLvlChange& best_lvl_change);
    void handle_after();
    void handle_before();
    void reset();

    OB::OrderBook<OB::VectorLevelBSearchSplit> order_book;

    bool touched = false;
    absl::flat_hash_map<uint64_t, uint64_t> latency_distribution;

    uint64_t total_messages = 0;
    unsigned aux_start, aux_end;

    uint64_t t0;

    bool last_message = false;

    uint32_t last_price = 0;
    std::vector<uint32_t> prices;

    bool should_stop() {
        return last_message;
    }

    BenchmarkOrderBook() {
        #ifndef PERF
        prices.reserve(60'000);
        prices.push_back(0);
        #endif
    }
};

inline void BenchmarkOrderBook::handle_before() {
    #ifndef PERF
    touched = false;
    t0 = __rdtscp(&aux_start);
    #endif
}

inline void BenchmarkOrderBook::handle_after() {
    uint32_t best_bid = order_book.best_bid().price;

    #ifdef PERF
    benchmark::DoNotOptimize(best_bid);
    #endif

    #ifndef PERF
    _mm_lfence();
    uint64_t t1 = __rdtscp(&aux_end);
    auto cycles = t1 - t0;

    if (touched) {
        latency_distribution[cycles]++;
    }
    #endif
}

inline void BenchmarkOrderBook::handle_change(const OB::BestLvlChange& best_lvl_change) {
    touched = true;
    total_messages++;

    if (best_lvl_change.side == OB::Side::Bid && best_lvl_change.price != prices.back() && !prices.empty()) {
        prices.push_back(best_lvl_change.price);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::SystemEvent& msg) {
    if (msg.event_code == 'C') {
        touched = true;
        last_message = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::StockDirectory& msg) {
    if (std::string_view(msg.stock, 8) == "NVDA    ") {
        target_stock_locate = msg.stock_locate;
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::AddOrderNoMpid& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.add_order(msg.order_reference_number, static_cast<OB::Side>(msg.buy_sell), msg.shares, msg.price);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::AddOrderMpid& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.add_order(msg.order_reference_number, static_cast<OB::Side>(msg.buy_sell), msg.shares, msg.price);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderExecuted& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.execute_order(msg.order_reference_number, msg.executed_shares);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderExecutedPrice& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.execute_order(msg.order_reference_number, msg.executed_shares);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderCancel& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.cancel_order(msg.order_reference_number, msg.cancelled_shares);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderDelete& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.delete_order(msg.order_reference_number);
        handle_change(change);
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderReplace& msg) {
    if (msg.stock_locate == target_stock_locate) {
        auto change = order_book.replace_order(msg.order_reference_number, msg.new_reference_number, msg.shares, msg.price);
        handle_change(change);
    }
}
