#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <iostream>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "itch_parser.hpp"
#include "benchmarks/example_benchmark.hpp"
#include "benchmarks/example_benchmark_parsing.hpp"

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
            "-e",
            "branch-misses",
            "-c", "100",
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

    double total_sec = 0;
    out << "latency_ns,count\n";
    for (const auto& [cycles, count] : data) {
        __int128 tmp = (__int128)cycles * 1'000'000'000;
        uint64_t ns = cycles * 1e9 / rdtscp_freq;
        double sec = double(cycles) / double(rdtscp_freq);
        total_sec += sec * count;
        out << ns << "," << count << "\n";
    }

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

    auto res = init_benchmark(filepath);
    auto src_buf = res.first;
    auto bytes_read = res.second;

    const std::byte* src = src_buf.data();
    size_t len = bytes_read;

    pid_t pid = run_perf_stat();
    sleep(3);

    ITCH::ItchParser parser;
    {
        BenchmarkOrderBook ob_bm_handler;
        parser.parse(src, len, ob_bm_handler);
        export_latency_distribution_csv(ob_bm_handler, outdir + "parsing_and_order_book_latency_distribution.csv");
        export_prices_csv(ob_bm_handler.prices, outdir);
    }

    //{
    //    BenchmarkParsing parsing_bm_handler;
    //    parser.parse(src, len, parsing_bm_handler);
    //    export_latency_distribution_csv(parsing_bm_handler, outdir + "parsing_lantecy_distribution.csv");
    //}

    kill(pid, SIGINT);
    return 0;
}
