#pragma once

#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include <x86intrin.h>
#include "levels/heap_level.hpp"
#include "itch_parser.hpp"
#include "order_book.hpp"

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

    void handle_after();
    void handle_before();
    void reset();

    OB::OrderBook<OB::HeapLevels> order_book;

    bool touched = false;
    absl::flat_hash_map<uint64_t, uint64_t> latency_distribution;

    uint64_t total_messages = 0;
    unsigned aux_start, aux_end;

    uint64_t t0;

    uint32_t last_price = 0;
    std::vector<uint32_t> prices;

    BenchmarkOrderBook() {
        prices.reserve(60'000);
    }
};

inline void BenchmarkOrderBook::handle_before() {
    touched = false;
    t0 = __rdtscp(&aux_start);
}

inline void BenchmarkOrderBook::handle_after() {
    uint32_t best_bid = order_book.best_bid().price;
    if (last_price != best_bid) {
        prices.push_back(best_bid);
        last_price = best_bid;
    }

    uint64_t t1 = __rdtscp(&aux_end);
    auto cycles = t1 - t0;

    if (touched && aux_end == aux_start) {
        latency_distribution[cycles]++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::StockDirectory& msg) {
    if (std::string_view(msg.stock, 8) == "NVDA    ") {
        target_stock_locate = msg.stock_locate;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::AddOrderNoMpid& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.add_order(msg.order_reference_number, static_cast<OB::Side>(msg.buy_sell), msg.shares, msg.price);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::AddOrderMpid& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.add_order(msg.order_reference_number, static_cast<OB::Side>(msg.buy_sell), msg.shares, msg.price);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderExecuted& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.execute_order(msg.order_reference_number, msg.executed_shares);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderExecutedPrice& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.execute_order(msg.order_reference_number, msg.executed_shares);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderCancel& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.cancel_order(msg.order_reference_number, msg.cancelled_shares);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderDelete& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.delete_order(msg.order_reference_number);
        touched = true;
        total_messages++;
    }
}

inline void BenchmarkOrderBook::handle(const ITCH::OrderReplace& msg) {
    if (msg.stock_locate == target_stock_locate) {
        order_book.replace_order(msg.order_reference_number, msg.new_reference_number, msg.shares, msg.price);
        touched = true;
        total_messages++;
    }
}
