#pragma once
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class HybridVectorMap {
public:
    HybridVectorMap() {
        prices.reserve(1000);
        qty_by_price.reserve(1000);
    }

    void add(Level level);
    void remove(Level level);
    Level best();

private:
    struct PriceCmp {
        bool operator()(uint32_t a, uint32_t b) const {
            if constexpr (S == Side::Bid) {
                return a < b;
            } else {
                return a > b;
            }
        }
    };

    std::vector<uint32_t> prices;
    absl::flat_hash_map<uint32_t, uint64_t> qty_by_price;
};

template<Side S>
inline void HybridVectorMap<S>::add(Level level) {
    auto& qty = qty_by_price[level.price];
    if (qty != 0) {
        qty += level.qty;
    } else [[unlikely]] {
        auto pit = std::lower_bound(
            prices.begin(), prices.end(), level.price,
            [](uint32_t lhs, uint32_t price) {
                if constexpr (S == Side::Bid) {
                    return lhs < price;
                } else {
                    return lhs > price;
                }
            }
        );
        prices.insert(pit, level.price);
        qty = level.qty;
    }
}

template<Side S>
inline void HybridVectorMap<S>::remove(Level level) {
    auto qit = qty_by_price.find(level.price);
    UNEXPECTED(qit == qty_by_price.end(), "Qty missing for level");
    UNEXPECTED(level.qty > qit->second, "Remove lead to an underflow");

    qit->second -= level.qty;
    if (qit->second == 0) [[unlikely]] {
        auto pit = std::lower_bound(
            prices.begin(), prices.end(), level.price,
            [](uint32_t lhs, uint32_t price) {
                if constexpr (S == Side::Bid) {
                    return lhs < price;
                } else {
                    return lhs > price;
                }
            }
        );
        UNEXPECTED(pit == prices.end() || *pit != level.price,
                   "Remove didn't find a level");
        qty_by_price.erase(qit);
        prices.erase(pit);
    }
}

template<Side S>
inline Level HybridVectorMap<S>::best() {
    if (!prices.empty()) {
        uint32_t price = prices.back();
        auto it = qty_by_price.find(price);
        UNEXPECTED(it == qty_by_price.end(), "Best price missing qty");
        return {price, it->second};
    } else {
        return {0, 0};
    }
}

}
