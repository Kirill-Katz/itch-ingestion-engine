#pragma once
#include <vector>
#include <algorithm>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class VectorLevelBSearchSplit {
public:
    VectorLevelBSearchSplit() {
        prices.reserve(5000);
        qtys.reserve(5000);
    }

    BestLvlChange remove(Level level);
    BestLvlChange add(Level level);

    Level best() const;

    std::vector<uint32_t> prices;
    std::vector<uint64_t> qtys;
};

template<Side S>
inline Level VectorLevelBSearchSplit<S>::best() const {
    if (!prices.empty()) {
        size_t i = prices.size() - 1;
        return { qtys[i], prices[i] };
    } else {
        return {0, 0};
    }
}

template<Side S>
inline BestLvlChange VectorLevelBSearchSplit<S>::remove(Level level) {
    auto it = std::lower_bound(
        prices.begin(), prices.end(), level.price,
        [](uint32_t lhs, uint32_t price) {
            if constexpr (S == Side::Bid) {
                return lhs < price;
            } else {
                return lhs > price;
            }
        }
    );

    UNEXPECTED(it == prices.end() || *it != level.price, "Remove didn't find a level");
    size_t idx = it - prices.begin();
    bool best_changed = (prices.size() - 1) == idx;

    UNEXPECTED(level.qty > qtys[idx], "Remove underflow");

    qtys[idx] -= level.qty;
    if (qtys[idx] == 0) {
        prices.erase(prices.begin() + idx);
        qtys.erase(qtys.begin() + idx);
    }

    return best_changed ? BestLvlChange{
        .qty = qtys.back(),
        .price = prices.back(),
        .side = S
    } : BestLvlChange{}; // return empty object as "no" change
    // no std::option used, because it provably doesn't return in registers
    // 0 for Side is defined as Side::None
}

template<Side S>
inline BestLvlChange VectorLevelBSearchSplit<S>::add(Level level) {
    auto it = std::lower_bound(
        prices.begin(), prices.end(), level.price,
        [](uint32_t lhs, uint32_t price) {
            if constexpr (S == Side::Bid) {
                return lhs < price;
            } else {
                return lhs > price;
            }
        }
    );

    size_t idx = it - prices.begin();
    bool best_changed = idx == (prices.size() - 1);

    if (it != prices.end() && *it == level.price) {
        qtys[idx] += level.qty;
    } else {
        prices.insert(it, level.price);
        qtys.insert(qtys.begin() + idx, level.qty);
    }

    return best_changed ? BestLvlChange{
        .qty = qtys.back(),
        .price = prices.back(),
        .side = S
    }
    : BestLvlChange{}; // same as above
}

}

