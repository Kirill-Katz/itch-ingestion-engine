#pragma once

#include <vector>
#include <string>
#include <sys/wait.h>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <fstream>

std::pair<std::vector<std::byte>, size_t> init_benchmark(std::string filename);
pid_t run_perf_report();
pid_t run_perf_stat();
uint64_t calibrate_tsc();
void export_prices_csv(const std::vector<uint32_t>& prices, std::string outdir);

inline uint64_t cycles_to_ns(uint64_t cycles, uint64_t freq) {
    __int128 num = (__int128)cycles * 1'000'000'000 + (freq / 2);
    return (uint64_t)(num / freq);
}


template<typename T>
void export_latency_distribution_csv(
    T& ob,
    std::string file_name
) {
    uint64_t rdtscp_freq = calibrate_tsc();
    std::cout << "rdtscp frequence: " << rdtscp_freq << '\n';

    std::vector<std::pair<uint64_t, uint64_t>> data;
    std::cout << "saved lantecies: " << ob.latency_distribution.size() << '\n';

    data.reserve(ob.latency_distribution.size());

    for (const auto& kv : ob.latency_distribution) {
        data.emplace_back(kv.first, kv.second);
    }

    std::sort(
        data.begin(),
        data.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    std::ofstream out(file_name);
    if (!out) {
        std::abort();
    }

    out << "latency_ns,count\n";
    for (const auto& [cycles, count] : data) {
        uint64_t ns = cycles_to_ns(cycles, rdtscp_freq);
        out << ns << "," << count << "\n";
    }

    __int128 total_cycles = 0;
    for (const auto& [cycles, count] : data) {
        total_cycles += (__int128)cycles * count;
    }
    double total_sec = (double)total_cycles / (double)rdtscp_freq;

    std::cout << "Total seconds spent: " << total_sec << '\n';
    std::cout << "Throughput: " << ob.total_messages / total_sec << '\n';
}

