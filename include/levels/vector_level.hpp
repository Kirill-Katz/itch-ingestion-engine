#pragma once
#include <vector>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class VectorLevel {
public:
    VectorLevel() {
        levels.reserve(3000);
    }

    void remove(Level level);
    void add(Level level);
    Level best() const;

    std::vector<Level> levels;
};

template<Side S>
inline Level VectorLevel<S>::best() const {
    return levels.back();
}

template<Side S>
void VectorLevel<S>::remove(Level level) {
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
void VectorLevel<S>::add(Level level) {
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

}
