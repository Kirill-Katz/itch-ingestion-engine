# General 
This is an ITCH parser which updates a custom order book implementations. Latency results can be seen below.
The spikes every 3ns are caused by the use or rdtspc instruction to measure latency, which on my laptop yields
to around 0.3 cycle per ns.

# How to use? 
### The ITCH 5.0 parser
The itch parser is a fully independent C++20 header file, which can be dragged and dropped into an existent project as it is.

The file can be found at ```include/itch_parser.hpp``` and can be accessed in the ITCH:: namespace.

An example of a Handler class can be found at ```include/benchmarks/example_benchmark_parsing.hpp``` and also in ```include/benchmarks/example_benchmark.hpp```.

The usage of both the parser and the handler can be found in ```src/main.cpp```.

### The Order Book
The order book requires the absl library in order to use the flat_hash_map.h header. It can be installed using the following commands: 
```
sudo apt update
sudo apt install libabsl-dev
```

The `order_book.hpp` implementation serves as an interface and all implementations for the underlying order book operations can be found in the `include/levels/` directory. The best implementation used in the benchmarks is the `include/levels/heap_level.hpp` implementation.

# How to run the benchmakrs?
Install `absl` and `google-benchmark`:
```
sudo apt update
sudo apt install libabsl-dev
sudo apt install libbenchmark-dev
```

Then build and run like this: 
```
mkdir build
cd build
cmake ..
make
./benchmark [path to the ITCH file] [results directory]
```

# How to analyze?
First install matplotlib by running:
```
pip install matplotlib
```

To analyze the latency you have to run the `analysis/plot_latency_distribution.py` and `analysis/plot_prices.py` files like this:
```
python plot_latency_distribution.py [input directory] [output directory]
python plot_prices.py [path to prices.csv] [output png file]
```
You can also analyze the benchmarks using perf by uncommenting the 3 lines in `int main()`, but make sure you remove the timing code from the benchmark handlers so that the perf is not poluted with almost perfectly predicted brances from the timing code.

# Where to get the ITCH file? 
The ITCH file can be downloaded here: https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/. For my tests I downloaded the 01302019.NASDAQ_ITCH50 file. Be aware that an ITCH file take around 10Gb.

# Results

The results were obtained on a pinned p-core of an i7-12700h CPU using `taskset -c 1` with turbo boost on (4.653Ghz peak) with Hyper Threading on and the CPU frequency scaling governor set to performance on an idle machine. The machine is an Asus ROG Zephyrus M16 GU603ZM_GU603ZM. The OS is Ubuntu 24.04.3 LTS with an unmodified Linux 6.14.0-37-generic kernel. Compiled with g++ 13.3.0 with -DNDEBUG -O3 -march=native flags. Latency measured using the `rdtscp` instruction and then converted into ns by estimating its frequence. The order book results were obtained on the first 3GB of the above mentioned file on the Nvidia stock messages using the `include/levels/heap_level.hpp` implementation. The results for the parser benchmark were obtained on all types of ITCH messages on the same first 3GB of the file.

### ITCH parsing + Order Book Updates Latency Distribution 
<img width="3000" height="1800" alt="parsing_and_order_book_latency_distribution" src="https://github.com/user-attachments/assets/7bd09120-943f-49e3-a60a-75e337617a59" />


### ITCH parsing Latency Distribution
<img width="3000" height="1800" alt="parsing_lantecy_distribution" src="https://github.com/user-attachments/assets/82497778-c3b8-466d-a183-7fbc7ff3ca8e" />

### Best Bids for Nvidia stock for the first few trading hours on 01.30.2019

<img width="1920" height="1440" alt="best_bids" src="https://github.com/user-attachments/assets/07fe6422-46ed-42ea-936d-2e5215921216" />

