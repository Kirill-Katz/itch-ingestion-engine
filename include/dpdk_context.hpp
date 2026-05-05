#pragma once

#include <rte_eal.h>
#include <rte_mbuf_core.h>

class DPDKContext {
public:
    DPDKContext(uint16_t port_id)
    : port_id_{port_id}
    {};

    void setup_eal(int& argc, char**& argv);
    void setup_mempool();
    void setup_eth_device(uint16_t port_id);

    uint16_t get_port_id() {
        return port_id_;
    }

    rte_mempool* get_pool() const {
        return pool_;
    }

private:
    rte_mempool* pool_;
    uint16_t port_id_;
};

