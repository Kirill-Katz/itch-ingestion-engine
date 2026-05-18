#pragma once

#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include <emmintrin.h>
#include <x86intrin.h>

#include "itch_parser.hpp"
#include "levels/vector_levels_b_search_split.hpp"
#include "order_book.hpp"
#include "order_book_shared.hpp"
#include "spmc_queue.hpp"

enum class StrategyMsgType : uint8_t {
    BookUpdate,
    Stop
};

struct BookUpdateMsg {
    uint64_t t0;
    uint64_t qty;
    uint32_t price;
    OB::Side side;
};

struct StopMsg {};

struct StrategyMsg {
    StrategyMsgType type;

    union {
        BookUpdateMsg book_update;
        StopMsg stop;
    };
};

class Handler {
public:
    using Book = OB::OrderBook<OB::VectorLevelBSearchSplit>;
    using Queue = SPMCQueue<StrategyMsg>;

    void handle(const ITCH::StockDirectory&);
    void handle(const ITCH::AddOrderNoMpid&);
    void handle(const ITCH::AddOrderMpid&);
    void handle(const ITCH::OrderExecuted&);
    void handle(const ITCH::OrderExecutedPrice&);
    void handle(const ITCH::OrderCancel&);
    void handle(const ITCH::OrderDelete&);
    void handle(const ITCH::OrderReplace&);
    void handle(const ITCH::SystemEvent&);

    void handle_change(const OB::BestLvlChange& best_lvl_change, uint64_t timestamp, Queue* queue);
    void handle_after();
    void handle_before();
    void reset();

    uint64_t t0;
    unsigned aux_start;

    bool last_message = false;
    bool record_prices = false;

    bool should_stop() {
        return last_message;
    }

    struct InstrumentConfig {
        std::string symbol;
        Queue* queue;
    };

    uint64_t max_orders = 0;

    Handler(const std::vector<InstrumentConfig>& instruments)
    {
        locate_to_book.fill(nullptr);
        locate_to_queue.fill(nullptr);

        for (const auto& cfg : instruments) {
            instruments_.emplace(pad_symbol(cfg.symbol), cfg);
        }
    }

private:
    std::unordered_map<std::string, InstrumentConfig> instruments_;

    static constexpr size_t max_locates_ = 65536;
    std::vector<std::unique_ptr<Book>> books;

    std::array<Book*, max_locates_> locate_to_book{};
    std::array<Queue*, max_locates_> locate_to_queue{};

    std::string pad_symbol(std::string_view);

    struct BookQueue {
        Book* book;
        Queue* queue;
    };

    BookQueue get_book_queue(uint16_t);
};

inline void Handler::handle_before() {
    t0 = __rdtscp(&aux_start);
}

inline void Handler::handle_after() {}

inline void Handler::handle_change(const OB::BestLvlChange& best_lvl_change, uint64_t timestamp, Queue* queue) {
    if (best_lvl_change.side == OB::Side::None) {
        return;
    }

    auto msg = StrategyMsg {
        .type = StrategyMsgType::BookUpdate,
        .book_update {
            .t0 = t0,
            .qty = best_lvl_change.qty,
            .price = best_lvl_change.price,
            .side = best_lvl_change.side
        }
    };

    queue->push(msg);
}

inline Handler::BookQueue Handler::get_book_queue(uint16_t stock_locate) {
    return {
        locate_to_book[stock_locate],
        locate_to_queue[stock_locate]
    };
}

inline void Handler::handle(const ITCH::SystemEvent& msg) {
    if (msg.event_code == 'C') { // last message
        last_message = true;
        for (auto q : locate_to_queue) {
            if (q != nullptr) {
                q->push({ .type = StrategyMsgType::Stop });
            }
        }
        std::cout << "Max orders: " << max_orders << '\n';
    } else if (msg.event_code == 'Q') { // start of market hours
        record_prices = true;
    } else if (msg.event_code == 'M') { // end of market hours
        record_prices = false;
    }
}

inline std::string Handler::pad_symbol(std::string_view symbol) {
    std::string out(8, ' ');
    std::memcpy(out.data(), symbol.data(), std::min<size_t>(symbol.size(), 8));
    return out;
}

inline void Handler::handle(const ITCH::StockDirectory& msg) {
    std::string sym(msg.stock, 8);

    auto it = instruments_.find(sym);
    if (it == instruments_.end() || locate_to_book[msg.stock_locate] != nullptr) {
        return;
    }

    books.emplace_back(std::make_unique<Book>());

    locate_to_book[msg.stock_locate] = books.back().get();
    locate_to_queue[msg.stock_locate] = it->second.queue;
}

inline void Handler::handle(const ITCH::AddOrderNoMpid& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->add_order(
        msg.order_reference_number,
        static_cast<OB::Side>(msg.buy_sell),
        msg.shares,
        msg.price
    );

    max_orders = std::max(max_orders, book->orders_map.size());

    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::AddOrderMpid& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->add_order(
        msg.order_reference_number,
        static_cast<OB::Side>(msg.buy_sell),
        msg.shares,
        msg.price
    );

    max_orders = std::max(max_orders, book->orders_map.size());

    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::OrderExecuted& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->execute_order(
        msg.order_reference_number,
        msg.executed_shares
    );
    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::OrderExecutedPrice& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->execute_order(
        msg.order_reference_number,
        msg.executed_shares
    );
    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::OrderCancel& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->cancel_order(
        msg.order_reference_number,
        msg.cancelled_shares
    );
    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::OrderDelete& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->delete_order(msg.order_reference_number);
    handle_change(change, msg.timestamp, queue);
}

inline void Handler::handle(const ITCH::OrderReplace& msg) {
    auto [book, queue] = get_book_queue(msg.stock_locate);
    if (book == nullptr) {
        return;
    }

    auto change = book->replace_order(
        msg.order_reference_number,
        msg.new_reference_number,
        msg.shares,
        msg.price
    );
    handle_change(change, msg.timestamp, queue);
}
