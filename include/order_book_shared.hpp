#pragma once

#include <cstdint>
#include <iostream>

namespace OB {
struct Level {
    uint64_t qty;
    uint32_t price;
};

enum class Side {
    Bid = 'B',
    Ask = 'S'
};

struct Order {
    uint32_t qty;
    uint32_t price;
    Side side;
};

[[gnu::noinline]] static void UNEXPECTED(bool condition, std::string_view message) {
    if (condition) {
        std::cerr << message << '\n';
        std::abort();
    }
}
}

