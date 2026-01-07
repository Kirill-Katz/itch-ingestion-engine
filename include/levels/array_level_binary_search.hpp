#pragma once

#include <array>
#include <string_view>
#include <algorithm>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class ArrayLevelBSearch {
public:
    static constexpr size_t capacity = 3000;

    void remove(Level level);
    void add(Level level);
    Level best() const;

    std::array<Level, capacity> levels{};
    size_t size = 0;
};

template<Side S>
inline Level ArrayLevelBSearch<S>::best() const {
    return levels[size - 1];
}

template<Side S>
inline void ArrayLevelBSearch<S>::remove(Level level) {
    auto b = levels.begin();
    auto e = b + size;

    auto it = std::lower_bound(
        b, e, level.price,
        [](const Level& lhs, uint32_t price) {
            if constexpr (S == Side::Bid) {
                return lhs.price < price;
            } else {
                return lhs.price > price;
            }
        }
    );

    UNEXPECTED(it == e || it->price != level.price, "Remove didn't find a level");
    UNEXPECTED(level.qty > it->qty, "Remove lead to an underflow for level");

    it->qty -= level.qty;
    if (it->qty == 0) {
        std::move(it + 1, e, it);
        --size;
    }
}

template<Side S>
inline void ArrayLevelBSearch<S>::add(Level level) {
    UNEXPECTED(size == capacity, "ArrayLevelBSearch capacity exceeded");

    auto b = levels.begin();
    auto e = b + size;

    auto it = std::lower_bound(
        b, e, level.price,
        [](const Level& lhs, uint32_t price) {
            if constexpr (S == Side::Bid) {
                return lhs.price < price;
            } else {
                return lhs.price > price;
            }
        }
    );

    if (it != e && it->price == level.price) { [[likely]]
        it->qty += level.qty;
    } else {
        std::move_backward(it, e, e + 1);
        *it = level;
        ++size;
    }
}

}

