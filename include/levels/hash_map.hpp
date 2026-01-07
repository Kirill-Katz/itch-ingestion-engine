#pragma once

#include <cstdint>
#include <memory>
#include <cstring>
#include "order_book_shared.hpp"


namespace OB {

class PriceToQtyMap {

struct Entry {
    uint64_t key;
    uint64_t value;
};

static constexpr uint64_t bucket_size = 128 / sizeof(Entry);
static constexpr uint64_t table_size = 8192;
static constexpr uint64_t buckets = table_size / bucket_size;

struct alignas(64) Bucket {
    Entry slots[bucket_size];
};

public:
    PriceToQtyMap() : table(std::make_unique<Bucket[]>(buckets)) {
        std::memset(table.get(), 0, sizeof(Bucket) * buckets);
    }

    static inline uint64_t hash(uint64_t x) {
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x;
    }

    inline void insert(uint64_t key, uint64_t value) {
        uint64_t bucket = hash(key) & (buckets - 1);

        for (int i = 0; i < bucket_size; ++i)  {
            if (table[bucket].slots[i].key == 0) {
                table[bucket].slots[i] = {key, value};
                return;
            }
        }

        UNEXPECTED(true, "Bucket full!");
    }

    inline uint64_t* find(uint64_t key) {
        uint64_t bucket = hash(key) & (buckets - 1);

        for (int i = bucket_size-1; i >= 0; --i)  {
            if (table[bucket].slots[i].key == key) {
                return &table[bucket].slots[i].value;
            }
        }

        return nullptr;
    }

    inline uint64_t operator[](uint64_t key) {
        uint64_t bucket = hash(key) & (buckets - 1);

        for (int i = 0; i < bucket_size; ++i)  {
            if (table[bucket].slots[i].key == key) {
                return table[bucket].slots[i].value;
            }
        }

        UNEXPECTED(true, "Value not found for key");
        return 0;
    }

    inline void erase(uint64_t key) {
        uint64_t bucket = hash(key) & (buckets - 1);

        for (int i = 0; i < bucket_size; ++i)  {
            if (key == table[bucket].slots[i].key) {
                table[bucket].slots[i].key = 0;
                return;
            }
        }

        UNEXPECTED(true, "Unable to delete, key not found!");
    }

private:
    std::unique_ptr<Bucket[]> table;

};

}
