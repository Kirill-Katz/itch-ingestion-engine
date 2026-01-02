#pragma once
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace OB {

struct Level {
    uint64_t price;
    uint64_t qty;
};

enum class Side {
    Bid = 'B',
    Ask = 'S'
};

template<Side S>
class VectorLevel {
public:
    void remove(Level level);
    void add(Level level);
    Level best() const;

    std::vector<Level> levels;
};

[[gnu::noinline]] static void UNEXPECTED(bool condition, std::string message) {
    if (condition) {
        std::cerr << message << '\n';
        std::abort();
    }
}

template<Side S>
inline Level VectorLevel<S>::best() const {
    return levels.back();
}

template<Side S>
inline void VectorLevel<S>::remove(Level level) {
    auto rit = std::ranges::find_if(
        levels.rbegin(), levels.rend(),
        [price = level.price](const Level& p) {
            return p.price == price;
        }
    );

    UNEXPECTED(rit == levels.rend(), "Remove didn't find a level");
    UNEXPECTED(level.qty > rit->qty, "Remove lead to an underflow for level");

    rit->qty -= level.qty;
    if (rit->qty == 0) {
        levels.erase(std::next(rit).base());
    }
}

template<Side S>
inline void VectorLevel<S>::add(Level level) {
    auto rit = std::ranges::find_if(
        levels.rbegin(), levels.rend(),
        [price = level.price](const Level& p) {
            if constexpr (S == Side::Bid) {
                return p.price <= price;
            } else {
                return p.price >= price;
            }
        }
    );

    if (rit != levels.rend() && rit->price == level.price) {
        rit->qty += level.qty;
    } else {
        levels.insert(rit.base(), level);
    }
}

struct Order {
    uint32_t qty;
    uint32_t price;
    Side side;
};

template<template<Side> typename Levels>
class OrderBook {
public:
    void add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price);
    void cancel_order(uint64_t order_id, uint32_t qty);
    void execute_order(uint64_t order_id, uint32_t qty);
    void replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price);
    void delete_order(uint64_t order_id);

    Level best_bid() const;
    Level best_ask() const;

    std::unordered_map<uint64_t, Order> orders_map;
    Levels<Side::Bid> bid_levels;
    Levels<Side::Ask> ask_levels;
};

template<template<Side> typename Levels>
inline Level OrderBook<Levels>::best_bid() const {
    return bid_levels.best();
}

template<template<Side> typename Levels>
inline Level OrderBook<Levels>::best_ask() const {
    return ask_levels.best();
}

template<template<Side> typename Levels>
inline void OrderBook<Levels>::add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price) {
    Order order;
    order.qty = qty;
    order.side = side;
    order.price = price;
    orders_map.insert({order_id, order});

    if (side == Side::Bid) {
        bid_levels.add({price, qty});
    } else {
        ask_levels.add({price, qty});
    }
}

template<template<Side> typename Levels>
inline void OrderBook<Levels>::cancel_order(uint64_t order_id, uint32_t qty) {
    Order& order = orders_map.at(order_id);

    UNEXPECTED(order.qty < qty, "Partial cancel order volume greater than order volume");

    if (order.side == Side::Bid) {
        bid_levels.remove({order.price, qty});
    } else {
        ask_levels.remove({order.price, qty});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
inline void OrderBook<Levels>::execute_order(uint64_t order_id, uint32_t qty) {
    Order& order = orders_map.at(order_id);

    UNEXPECTED(order.qty < qty, "Partial execute order volume greater than order volume");

    if (order.side == Side::Bid) {
        bid_levels.remove({order.price, qty});
    } else {
        ask_levels.remove({order.price, qty});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
inline void OrderBook<Levels>::replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price) {
    Order& old_order = orders_map.at(order_id);

    Order new_order;
    new_order.side = old_order.side;
    new_order.price = price;
    new_order.qty = qty;
    orders_map.insert({new_order_id, new_order});

    if (new_order.side == Side::Bid) {
        bid_levels.remove({old_order.price, old_order.qty});
        bid_levels.add({price, qty});
    } else {
        ask_levels.remove({old_order.price, old_order.qty});
        ask_levels.add({price, qty});
    }

    orders_map.erase(order_id);
}

template<template<Side> typename Levels>
inline void OrderBook<Levels>::delete_order(uint64_t order_id) {
    Order& order = orders_map.at(order_id);

    if (order.side == Side::Bid) {
        bid_levels.remove({order.price, order.qty});
    } else {
        ask_levels.remove({order.price, order.qty});
    }

    orders_map.erase(order_id);
}

}
