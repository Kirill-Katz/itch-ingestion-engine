#pragma once
#include <queue>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"
#include "hash_map.hpp"

namespace OB {

template<Side S>
class HeapLevelsCustomMap {
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
    //absl::flat_hash_map<uint32_t, uint64_t> qty_by_price;
    PriceToQtyMap qty_by_price;
};

template<Side S>
inline void HeapLevelsCustomMap<S>::add(Level level) {
    uint64_t* qty = qty_by_price.find(level.price);
    if (qty == nullptr) {
        qty_by_price.insert(level.price, level.qty);
        heap.push(level.price);
        return;
    }

    *qty += level.qty;
}

template<Side S>
inline void HeapLevelsCustomMap<S>::remove(Level level) {
    uint64_t* qty = qty_by_price.find(level.price);
    UNEXPECTED(qty == nullptr, "Remove didn't find level");
    UNEXPECTED(level.qty > *qty, "Remove underflow");

    *qty -= level.qty;
    if (*qty == 0) {
        qty_by_price.erase(level.price);
    }
}

template<Side S>
inline Level HeapLevelsCustomMap<S>::best() {
    while (!heap.empty()) {
        uint32_t price = heap.top();
        uint64_t* qty = qty_by_price.find(price);
        if (qty != nullptr && *qty) {
            return Level{ price, *qty };
        }
        heap.pop();
    }

    //UNEXPECTED(true, "Best called on empty book");
    return {0, 0};
}

}
