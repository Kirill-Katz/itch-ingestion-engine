#pragma once
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <cstring>
#include <sys/mman.h>
#include "order_book_shared.hpp"

namespace OB {

template<Side S>
class ArrayLevelsV2 {
public:
    void add(Level level);
    void remove(Level level);
    Level best();

    // define the copy and move ops...

    ArrayLevelsV2() : book(static_cast<uint64_t*>(std::aligned_alloc(64, N * sizeof(uint64_t)))) {
        std::memset(book, 0, N * sizeof(uint64_t));
        best_idx = (S == Side::Bid) ? 0 : N - 1;
    }

    ~ArrayLevelsV2() {
        std::free(book);
    }

private:
    static constexpr int N = 2'000'000;
    uint64_t* book;
    uint32_t best_idx = 0;
};

template<Side S>
inline void ArrayLevelsV2<S>::add(Level level) {
    if (level.price > N) {
        return;
    }

    book[level.price] += level.qty;
    if constexpr (S == Side::Bid) {
        if (level.price > best_idx) {
            best_idx = level.price;
        }
    } else {
        if (level.price < best_idx) {
            best_idx = level.price;
        }
    }
}

template<Side S>
inline void ArrayLevelsV2<S>::remove(Level level) {
    if (level.price > N) {
        return;
    }

    auto& qty = book[level.price];
    UNEXPECTED(level.qty > qty, "Remove underflow");

    qty -= level.qty;
    if (qty == 0 && level.price == best_idx) {
        if constexpr (S == Side::Bid) {
            while (best_idx > 0 && book[best_idx] == 0) {
                best_idx--;
            }
        } else {
            while (best_idx < N-1 && book[best_idx] == 0) {
                best_idx++;
            }
        }
    }
}

template<Side S>
inline Level ArrayLevelsV2<S>::best() {
    auto qty = book[best_idx];
    return {best_idx, qty};
}

}

