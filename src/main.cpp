#include <iostream>

#include <rte_ether.h>
#include <rte_ip4.h>
#include <rte_udp.h>
#include <vector>
#include <iostream>
#include <time.h>
#include <rte_eal.h>
#include <rte_ethdev.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <thread>
#include <string_view>

#include "itch_parser.hpp"
#include "benchmarks/benchmark_utils.hpp"
#include "benchmarks/example_benchmark.hpp"
#include "benchmarks/example_benchmark_parsing.hpp"
#include "dpdk_context.hpp"
#include "ingestor.hpp"
#include "handler.hpp"
#include "spmc_queue.hpp"

struct StrategyConsumerConfig {
    std::string name;
    Handler::Queue* queue;
    int cpu;
};

static void add_strategy_consumers(
    std::vector<StrategyConsumerConfig>& consumers,
    std::string_view prefix,
    Handler::Queue& queue,
    const std::vector<int>& cpus
) {
    for (size_t i = 0; i < cpus.size(); ++i) {
        consumers.push_back({
            .name = std::string(prefix) + "_consumer_" + std::to_string(i + 1),
            .queue = &queue,
            .cpu = cpus[i]
        });
    }
}

int main(int argc, char** argv) {
    cpu_set_t main_cpuset;
    CPU_ZERO(&main_cpuset);
    CPU_SET(1, &main_cpuset);
    int main_pin_result = pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set_t),
        &main_cpuset
    );
    if (main_pin_result != 0) {
        std::cerr << "Failed to pin main thread to core 1\n";
        return 1;
    }

    constexpr uint16_t port_id = 0;
    DPDKContext dpdk_context(port_id);

    dpdk_context.setup_eal(argc, argv);
    dpdk_context.setup_mempool();
    dpdk_context.setup_eth_device(port_id);

    std::string outdir;

    if (argc != 2) {
        std::cout << "Please specify the file to parse and an output directory" << '\n';
        return 1;
    }

    outdir = argv[1];

    ITCH::ItchParser parser;
    BenchmarkOrderBook ob_bm_handler;
    BenchmarkParsing parsing_bm_handler;

    std::vector<Handler::InstrumentConfig> instrument_config;
    Handler::Queue nvda_queue;
    Handler::Queue aapl_queue;

    instrument_config.push_back({ .symbol = "NVDA", .queue = &nvda_queue });
    instrument_config.push_back({ .symbol = "AAPL", .queue = &aapl_queue });

    std::vector<StrategyConsumerConfig> consumer_configs;
    add_strategy_consumers(consumer_configs, "nvidia", nvda_queue, {2});
    add_strategy_consumers(consumer_configs, "apple", aapl_queue, {4, 5});

    std::vector<std::thread> consumer_threads;
    consumer_threads.reserve(consumer_configs.size());

    for (const auto& consumer_cfg : consumer_configs) {
        auto consumer = consumer_cfg.queue->make_consumer();
        consumer_threads.emplace_back([c = std::move(consumer), o = outdir, name = consumer_cfg.name] () mutable {
            absl::flat_hash_map<uint64_t, uint64_t> latency_distribution;

            while (true) {
                StrategyMsg msg;
                unsigned aux_end;

                while (!c.pop(msg)) {}
                if (msg.type == StrategyMsgType::Stop) {
                    break;
                }

                _mm_lfence();
                uint64_t t1 = __rdtscp(&aux_end);
                auto cycles = t1 - msg.book_update.t0;

                latency_distribution[cycles]++;
            }

            export_latency_distribution_csv_cycles(
                latency_distribution,
                o + name + "_latency_distribution.csv"
            );
        });

        cpu_set_t consumer_cpuset;
        CPU_ZERO(&consumer_cpuset);
        CPU_SET(consumer_cfg.cpu, &consumer_cpuset);
        int consumer_pin_result = pthread_setaffinity_np(
            consumer_threads.back().native_handle(),
            sizeof(cpu_set_t),
            &consumer_cpuset
        );
        if (consumer_pin_result != 0) {
            std::cerr << "Failed to pin " << consumer_cfg.name
                      << " to core " << consumer_cfg.cpu << "\n";
            return 1;
        }
    }

    Handler handler(instrument_config);

    ITCH::Ingestor<Handler> ingestor(handler, dpdk_context);
    ingestor.ingest_messages();

    for (auto& consumer_thread : consumer_threads) {
        consumer_thread.join();
    }
    return 0;
}
