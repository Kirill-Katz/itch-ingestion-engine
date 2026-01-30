#pragma once
#include <cstdint>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"
#include "spsc_queue.hpp"

namespace OB {

template<template<Side> typename Levels>
class SingleStartOrderBook {
public:
    void add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price);
    void cancel_order(uint64_t order_id, uint32_t qty);
    void execute_order(uint64_t order_id, uint32_t qty);
    void replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price);
    void delete_order(uint64_t order_id);

    Level best_bid();
    Level best_ask();

    SingleStartOrderBook(SPSCQueue<BestLvlChange>& change);

    uint64_t max_orders = 0;
    absl::flat_hash_map<uint64_t, Order> orders_map;
    Levels<Side::Bid> bid_levels;
    Levels<Side::Ask> ask_levels;

    SPSCQueue<BestLvlChange>& strat_queue;
};

template<template<Side> typename Levels>
 Level SingleStartOrderBook<Levels>::best_bid() {
    return bid_levels.best();
}

template<template<Side> typename Levels>
 Level SingleStartOrderBook<Levels>::best_ask() {
    return ask_levels.best();
}

template<template<Side> typename Levels>
void SingleStartOrderBook<Levels>::add_order(uint64_t order_id, Side side, uint32_t qty, uint32_t price) {
    Order order;
    order.qty = qty;
    order.side = side;
    order.price = price;
    orders_map.insert({order_id, order});

    BestLvlChange best_lvl_change{};

    if (side == Side::Bid) {
        best_lvl_change = bid_levels.add({qty, price});
    } else {
        best_lvl_change = ask_levels.add({qty, price});
    }

    if (best_lvl_change.side != Side::None) {
        bool res = strat_queue.try_push(best_lvl_change);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (add order)");
    }
}

template<template<Side> typename Levels>
void SingleStartOrderBook<Levels>::cancel_order(uint64_t order_id, uint32_t qty) {
    UNEXPECTED(!orders_map.contains(order_id), "Cancel order did not find an order");
    Order& order = orders_map.at(order_id);
    UNEXPECTED(order.qty < qty, "Partial cancel order volume greater than order volume");

    BestLvlChange best_lvl_change{};

    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({qty, order.price});
    }

    if (best_lvl_change.side != Side::None) {
        bool res = strat_queue.try_push(best_lvl_change);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (cancel order)");
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
void SingleStartOrderBook<Levels>::execute_order(uint64_t order_id, uint32_t qty) {
    UNEXPECTED(!orders_map.contains(order_id), "Execute order did not find an order");
    Order& order = orders_map.at(order_id);
    UNEXPECTED(order.qty < qty, "Partial execute order volume greater than order volume");

    BestLvlChange best_lvl_change{};

    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({qty, order.price});
    }

    if (best_lvl_change.side != Side::None) {
        bool res = strat_queue.try_push(best_lvl_change);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (execute order)");
    }

    order.qty -= qty;
    if (order.qty == 0) {
        orders_map.erase(order_id);
    }
}

template<template<Side> typename Levels>
void SingleStartOrderBook<Levels>::replace_order(uint64_t order_id, uint64_t new_order_id, uint32_t qty, uint32_t price) {
    UNEXPECTED(!orders_map.contains(order_id), "Replace order did not find an order");
    Order& old_order = orders_map.at(order_id);

    Order new_order;
    new_order.side = old_order.side;
    new_order.price = price;
    new_order.qty = qty;
    orders_map.insert({new_order_id, new_order});

    BestLvlChange op1{};
    BestLvlChange op2{};

    if (new_order.side == Side::Bid) {
        op1 = bid_levels.remove({old_order.qty, old_order.price});
        op2 = bid_levels.add({qty, price});
    } else {
        op1 = ask_levels.remove({old_order.qty, old_order.price});
        op2 = ask_levels.add({qty, price});
    }

    if (op2.side != Side::None) {
        bool res = strat_queue.try_push(op2);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (replace order)");
    } else if (op1.side != Side::None) {
        bool res = strat_queue.try_push(op1);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (replace order)");
    }

    orders_map.erase(order_id);
}

template<template<Side> typename Levels>
void SingleStartOrderBook<Levels>::delete_order(uint64_t order_id) {
    UNEXPECTED(!orders_map.contains(order_id), "Delete order did not find an order");
    Order& order = orders_map.at(order_id);

    BestLvlChange best_lvl_change{};

    if (order.side == Side::Bid) {
        best_lvl_change = bid_levels.remove({order.qty, order.price});
    } else {
        best_lvl_change = ask_levels.remove({order.qty, order.price});
    }

    if (best_lvl_change.side != Side::None) {
        bool res = strat_queue.try_push(best_lvl_change);
        UNEXPECTED(!res, "Strategy queue blocked, strategy is too slow (delete order)");
    }

    orders_map.erase(order_id);
}

}

