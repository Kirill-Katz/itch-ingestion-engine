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

    void remove(Level level);
    void add(Level level);
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
inline void VectorLevelBSearchSplit<S>::remove(Level level) {
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
    UNEXPECTED(level.qty > qtys[idx], "Remove underflow");

    qtys[idx] -= level.qty;

    if (qtys[idx] == 0) {
        prices.erase(prices.begin() + idx);
        qtys.erase(qtys.begin() + idx);
    }
}

template<Side S>
inline void VectorLevelBSearchSplit<S>::add(Level level) {
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

    if (it != prices.end() && *it == level.price) {
        qtys[idx] += level.qty;
    } else {
        prices.insert(it, level.price);
        qtys.insert(qtys.begin() + idx, level.qty);
    }
}

}

