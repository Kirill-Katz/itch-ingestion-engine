#pragma once
#include <queue>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class HeapLevels {
public:
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

    std::priority_queue<uint32_t, std::vector<uint32_t>, PriceCmp> heap;
    absl::flat_hash_map<uint32_t, uint64_t> qty_by_price;
};

template<Side S>
inline void HeapLevels<S>::add(Level level) {
    auto& qty = qty_by_price[level.price];
    if (qty == 0) {
        heap.push(level.price);
    }
    qty += level.qty;
}

template<Side S>
inline void HeapLevels<S>::remove(Level level) {
    auto it = qty_by_price.find(level.price);
    UNEXPECTED(it == qty_by_price.end(), "Remove didn't find level");
    UNEXPECTED(level.qty > it->second, "Remove underflow");

    it->second -= level.qty;
    if (it->second == 0) {
        qty_by_price.erase(it);
    }
}

template<Side S>
inline Level HeapLevels<S>::best() {
    while (!heap.empty()) {
        uint32_t price = heap.top();
        auto it = qty_by_price.find(price);
        if (it != qty_by_price.end() && it->second) {
            return Level{ price, it->second };
        }
        heap.pop();
    }

    return {0, 0};
}

}
