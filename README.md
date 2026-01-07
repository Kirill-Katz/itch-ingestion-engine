# General 
This is an ITCH parser which updates a custom order book implementations. Latency results can be seen below.
The spikes every 3ns are caused by the use or rdtspc instruction to measure latency, which on my laptop yields
to around 0.3 cycle per ns.

# How to use? 
### The ITCH 5.0 parser
The itch parser is a fully independet C++20 header file, which can be dragged and dropped into an existent project as it is.

The file can be found at ```include/itch_parser.hpp``` and can be accessed in the ITCH:: namespace.

An example of a Handler class can be found at ```include/example_benchmark_parsing.hpp```.

The usage of both the parser and the handler can be found in ```src/main.cpp```.

# Results

The results were obtained on a pinned p-core of an i7-12700h CPU using `taskset -c 1` with turbo boost on (4.653Ghz peak) with Hyper Threading on and the CPU frequency scaling governor set to performance on an idle machine. The machine is an Asus ROG Zephyrus M16 GU603ZM_GU603ZM. The OS is Ubuntu 24.04.3 LTS with an unmodified Linux 6.14.0-37-generic kernel. Compiled with g++ 13.3.0 with -DNDEBUG -O3 -march=native flags. Latency measured using the `rdtscp` instruction and then converted into ns by estimating its frequence. 

### ITCH parsing + Order Book Updates Latency Distribution 
<img width="3000" height="1800" alt="parsing_and_order_book_latency_distribution" src="https://github.com/user-attachments/assets/7bd09120-943f-49e3-a60a-75e337617a59" />


### ITCH parsing Latency Distribution
<img width="3000" height="1800" alt="parsing_lantecy_distribution" src="https://github.com/user-attachments/assets/c10d683f-3ff1-414b-b4f1-08fbfa188591" />


