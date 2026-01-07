#pragma once

#include <array>
#include <string_view>
#include <iterator>
#include <algorithm>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class ArrayLevel {
public:
    static constexpr size_t capacity = 3000;

    void remove(Level level);
    void add(Level level);
    Level best() const;

    std::array<Level, capacity> levels{};
    size_t size = 0;
};

template<Side S>
inline Level ArrayLevel<S>::best() const {
    return levels[size - 1];
}

template<Side S>
inline void ArrayLevel<S>::remove(Level level) {
    auto b = levels.begin();
    auto e = b + size;

    auto rb = std::make_reverse_iterator(e);
    auto re = std::make_reverse_iterator(b);

    auto rit = std::ranges::find_if(
        rb, re,
        [price = level.price](const Level& p) {
            return p.price == price;
        }
    );

    UNEXPECTED(rit == re, "Remove didn't find a level");

    auto it = std::next(rit).base();

    UNEXPECTED(level.qty > it->qty, "Remove lead to an underflow for level");

    it->qty -= level.qty;
    if (it->qty == 0) {
        std::move(it + 1, e, it);
        --size;
    }
}

template<Side S>
inline void ArrayLevel<S>::add(Level level) {
    UNEXPECTED(size == capacity, "ArrayLevel capacity exceeded");

    auto b = levels.begin();
    auto e = b + size;

    auto it = std::ranges::find_if(
        b, e,
        [price = level.price](const Level& p) {
            if constexpr (S == Side::Bid) {
                return p.price >= price;
            } else {
                return p.price <= price;
            }
        }
    );

    if (it != e && it->price == level.price) {
        it->qty += level.qty;
        return;
    }

    std::move_backward(it, e, e + 1);
    *it = level;
    ++size;
}

}

