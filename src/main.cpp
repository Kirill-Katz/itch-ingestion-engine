#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <iostream>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <thread>
#include <random>

#include "itch_parser.hpp"
#include "benchmarks/example_benchmark.hpp"
#include "benchmarks/example_benchmark_parsing.hpp"

std::atomic<bool> run_noise = true;

void allocator_noise() {
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> size_dist(4096, 8*4096);
    std::uniform_int_distribution<int> action(0, 1);

    std::vector<void*> blocks;

    while (run_noise.load()) {
        if (action(rng) == 0 || blocks.empty()) {
            size_t sz = size_dist(rng);
            void* p = std::malloc(sz);
            memset(p, 0xAA, sz);
            blocks.push_back(p);
        } else {
            size_t i = rng() % blocks.size();
            std::free(blocks[i]);
            blocks.erase(blocks.begin() + i);
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
    }
}

std::pair<std::vector<std::byte>, size_t> init_benchmark(std::string filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file\n";
        return {};
    }

    std::vector<std::byte> src_buf;
    src_buf.resize(3LL * 1024 * 1024 * 1024);

    file.read(reinterpret_cast<char*>(src_buf.data()), src_buf.size());
    size_t bytes_read = size_t(file.gcount());

    if (bytes_read < 3) {
        std::cerr << "File read too small\n";
        return {};
    }

    return {src_buf, bytes_read};
}

pid_t run_perf_report() {
    pid_t pid = fork();
    if (pid == 0) {
        char pidbuf[32];
        snprintf(pidbuf, sizeof(pidbuf), "%d", getppid());

        execlp(
            "perf",
            "perf",
            "record",
            "-F",
            "999",
            "-g",
            "-p", pidbuf,
            nullptr
        );

        _exit(127);
    }

    return pid;
}

pid_t run_perf_stat() {
    pid_t pid = fork();
    if (pid == 0) {
        char pidbuf[32];
        snprintf(pidbuf, sizeof(pidbuf), "%d", getppid());

        execlp(
            "perf",
            "perf",
            "stat",
            "-p", pidbuf,
            nullptr
        );

        _exit(127);
    }

    return pid;
}

uint64_t cycles_to_ns(uint64_t cycles, uint64_t freq) {
    __int128 num = (__int128)cycles * 1'000'000'000 + (freq / 2);
    return (uint64_t)(num / freq);
}

uint64_t calibrate_tsc() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t t0_ns = ts.tv_sec * 1'000'000'000ull + ts.tv_nsec;

    unsigned aux;
    uint64_t c0 = __rdtscp(&aux);

    timespec sleep_ts;
    sleep_ts.tv_sec = 1;
    sleep_ts.tv_nsec = 0;

    nanosleep(&sleep_ts, nullptr);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t t1_ns = ts.tv_sec * 1'000'000'000ull + ts.tv_nsec;

    uint64_t c1 = __rdtscp(&aux);

    uint64_t delta_cycles = c1 - c0;
    uint64_t delta_ns = t1_ns - t0_ns;

    __int128 tmp = (__int128)delta_cycles * 1'000'000'000;
    return (tmp + delta_ns / 2) / delta_ns;
}

template<typename T>
void export_latency_distribution_csv(
    T& ob,
    std::string file_name
) {
    uint64_t rdtscp_freq = calibrate_tsc();
    std::cout << "rdtscp frequence: " << rdtscp_freq << '\n';

    std::vector<std::pair<uint64_t, uint64_t>> data;
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

void export_prices_csv(
    const std::vector<uint32_t>& prices,
    std::string outdir
) {
    std::vector<uint32_t> data = prices;

    std::ofstream out(outdir + "prices.csv");
    if (!out) {
        std::abort();
    }

    out << "price\n";
    for (uint32_t price : data) {
        out << price << "\n";
    }
}

int main(int argc, char** argv) {
    std::string filepath;
    std::string outdir;

    if (argc != 3) {
        std::cout << "Please specify the file to parse and an output directory" << '\n';
        return 1;
    }

    filepath = argv[1];
    outdir   = argv[2];

    auto [src_buf, bytes_read] = init_benchmark(filepath);
    const std::byte* src = src_buf.data();
    size_t len = bytes_read;

    std::thread noise(allocator_noise);

    #ifdef PERF
        pid_t pid = run_perf_stat();
        sleep(3);
    #endif

    ITCH::ItchParser parser;
    {
        BenchmarkOrderBook ob_bm_handler;
        parser.parse(src, len, ob_bm_handler);

        #ifndef PERF
        export_latency_distribution_csv(ob_bm_handler, outdir + "parsing_and_order_book_latency_distribution.csv");
        export_prices_csv(ob_bm_handler.prices, outdir);
        #endif
    }

    {
        BenchmarkParsing parsing_bm_handler;
        parser.parse(src, len, parsing_bm_handler);

        #ifndef PERF
        export_latency_distribution_csv(parsing_bm_handler, outdir + "parsing_lantecy_distribution.csv");
        #endif
    }

    #ifdef PERF
        kill(pid, SIGINT);
    #endif

    run_noise = false;
    noise.join();

    return 0;
}
