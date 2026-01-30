#pragma once
#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"

namespace OB {

template<template<Side> typename Levels>
class OrderBook {
public:
    void add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price);
    void cancel_order(uint64_t order_id, uint32_t qty);
    void execute_order(uint64_t order_id, uint32_t qty);
    void replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price);
    void delete_order(uint64_t order_id);

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
void OrderBook<Levels>::add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price) {
    Order order;
    order.qty = qty;
    order.side = side;
    order.price = price;
    orders_map.insert({order_id, order});

    if (side == Side::Bid) {
        bid_levels.add({qty, price});
    } else {
        ask_levels.add({qty, price});
    }
}

template<template<Side> typename Levels>
void OrderBook<Levels>::cancel_order(uint64_t order_id, uint32_t qty) {
    UNEXPECTED(!orders_map.contains(order_id), "Cancel order did not find an order");
    Order& order = orders_map.at(order_id);
    UNEXPECTED(order.qty < qty, "Partial cancel order volume greater than order volume");

    if (order.side == Side::Bid) {
        bid_levels.remove({qty, order.price});
    } else {
        ask_levels.remove({qty, order.price});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
void OrderBook<Levels>::execute_order(uint64_t order_id, uint32_t qty) {
    UNEXPECTED(!orders_map.contains(order_id), "Execute order did not find an order");
    Order& order = orders_map.at(order_id);
    UNEXPECTED(order.qty < qty, "Partial execute order volume greater than order volume");

    if (order.side == Side::Bid) {
        bid_levels.remove({qty, order.price});
    } else {
        ask_levels.remove({qty, order.price});
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
void OrderBook<Levels>::replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price) {
    UNEXPECTED(!orders_map.contains(order_id), "Replace order did not find an order");
    Order& old_order = orders_map.at(order_id);

    Order new_order;
    new_order.side = old_order.side;
    new_order.price = price;
    new_order.qty = qty;
    orders_map.insert({new_order_id, new_order});

    if (new_order.side == Side::Bid) {
        bid_levels.remove({old_order.qty, old_order.price});
        bid_levels.add({qty, price});
    } else {
        ask_levels.remove({old_order.qty, old_order.price});
        ask_levels.add({qty, price});
    }

    orders_map.erase(order_id);
}

template<template<Side> typename Levels>
void OrderBook<Levels>::delete_order(uint64_t order_id) {
    UNEXPECTED(!orders_map.contains(order_id), "Delete order did not find an order");

    Order& order = orders_map.at(order_id);

    if (order.side == Side::Bid) {
        bid_levels.remove({order.qty, order.price});
    } else {
        ask_levels.remove({order.qty, order.price});
    }

    orders_map.erase(order_id);
}

}
