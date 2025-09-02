#pragma once

#include "Order.hpp"
#include "ExecutionReport.hpp"
#include <unordered_map>
#include <string>
#include <atomic>

namespace TradingEngine {

class OrderManager {
public:
    OrderManager();

    uint64_t add_new_order(Order& order);

    void update_order_status(const ExecutionReport& report);

    Order get_order(uint64_t order_id) const;

    double get_position(const std::string& symbol) const;

private:
    std::atomic<uint64_t> m_next_order_id;
    std::unordered_map<uint64_t, Order> m_orders;
    std::unordered_map<std::string, double> m_positions;
};

}
