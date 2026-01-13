#pragma once
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class BTreeLevels {
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

    absl::btree_map<
        uint32_t,
        uint64_t,
        std::conditional_t<
            S == Side::Bid,
            std::greater<uint32_t>,
            std::less<uint32_t>
        >
    > book;
};

template<Side S>
inline void BTreeLevels<S>::add(Level level) {
    book[level.price] += level.qty;
}

template<Side S>
inline void BTreeLevels<S>::remove(Level level) {
    auto it = book.find(level.price);
    UNEXPECTED(it == book.end(), "Remove didn't find level");
    UNEXPECTED(level.qty > it->second, "Remove underflow");

    it->second -= level.qty;
    if (it->second == 0) {
        //book.erase(it);
    }
}

template<Side S>
inline Level BTreeLevels<S>::best() {
    if (book.empty()) {
        return {0, 0};
    }

    auto it = book.begin();
    return {it->first, it->second};
}

}

