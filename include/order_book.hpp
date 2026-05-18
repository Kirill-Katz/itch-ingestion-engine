#pragma once
#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"

namespace OB {

template<template<Side> typename Levels>
class OrderBook {
public:
    BestLvlChange add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price);
    BestLvlChange cancel_order(uint64_t order_id, uint32_t qty);
    BestLvlChange execute_order(uint64_t order_id, uint32_t qty);
    BestLvlChange replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price);
    BestLvlChange delete_order(uint64_t order_id);

    Level best_bid();
    Level best_ask();

    uint64_t max_orders = 0;
    absl::flat_hash_map<uint64_t, Order> orders_map;
    Levels<Side::Bid> bid_levels;
    Levels<Side::Ask> ask_levels;
};

template<template<Side> typename Levels>
 Level OrderBook<Levels>::best_bid() {
    return bid_levels.best();
}

template<template<Side> typename Levels>
 Level OrderBook<Levels>::best_ask() {
    return ask_levels.best();
}

template<template<Side> typename Levels>
BestLvlChange OrderBook<Levels>::add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price) {
    Order order;
    order.qty = qty;
    order.side = side;
    order.price = price;
    orders_map.insert({order_id, order});

    BestLvlChange best_lvl_change;
    if (side == Side::Bid) {
        best_lvl_change = bid_levels.add({qty, price});
    } else {
        best_lvl_change = ask_levels.add({qty, price});
    }

    return best_lvl_change;
}

template<template<Side> typename Levels>
BestLvlChange OrderBook<Levels>::cancel_order(uint64_t order_id, uint32_t qty) {
    auto it = orders_map.find(order_id);
    UNEXPECTED(it == orders_map.end(), "Cancel order did not find an order");
    Order& order = it->second;
    UNEXPECTED(order.qty < qty, "Partial cancel order volume greater than order volume");

    BestLvlChange best_lvl_change;
    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({qty, order.price});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(it);
    }

    return best_lvl_change;
}

template<template<Side> typename Levels>
BestLvlChange OrderBook<Levels>::execute_order(uint64_t order_id, uint32_t qty) {
    auto it = orders_map.find(order_id);
    UNEXPECTED(it == orders_map.end(), "Execute order did not find an order");
    Order& order = it->second;
    UNEXPECTED(order.qty < qty, "Partial execute order volume greater than order volume");

    BestLvlChange best_lvl_change;
    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({qty, order.price});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(it);
    }

    return best_lvl_change;
}

template<template<Side> typename Levels>
BestLvlChange OrderBook<Levels>::replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price) {
    auto it = orders_map.find(order_id);
    UNEXPECTED(it == orders_map.end(), "Replace order did not find an order");
    Order& old_order = it->second;

    Order new_order;
    new_order.side = old_order.side;
    new_order.price = price;
    new_order.qty = qty;

    BestLvlChange best_change_rem{};
    BestLvlChange best_change_add{};

    if (new_order.side == Side::Bid) {
        best_change_rem = bid_levels.remove({old_order.qty, old_order.price});
        best_change_add = bid_levels.add({qty, price});
    } else {
        best_change_rem = ask_levels.remove({old_order.qty, old_order.price});
        best_change_add = ask_levels.add({qty, price});
    }

    orders_map.erase(it);
    orders_map.emplace(new_order_id, new_order);

    if (best_change_add.side != Side::None) {
        return best_change_add;
    } else return best_change_rem;
}

template<template<Side> typename Levels>
BestLvlChange OrderBook<Levels>::delete_order(uint64_t order_id) {
    auto it = orders_map.find(order_id);
    UNEXPECTED(it == orders_map.end(), "Delete order did not find an order");

    Order& order = it->second;

    BestLvlChange best_lvl_change;
    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({order.qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({order.qty, order.price});
    }

    orders_map.erase(it);
    return best_lvl_change;
}

}
