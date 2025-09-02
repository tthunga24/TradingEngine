#include "OrderManager.hpp"
#include "LogHandler.hpp"

namespace TradingEngine {

OrderManager::OrderManager() : m_next_order_id(1) {}

uint64_t OrderManager::add_new_order(Order& order) {
    uint64_t id = m_next_order_id++;
    order.order_id = id;
    m_orders[id] = order;
    return id;
}

void OrderManager::update_order_status(const ExecutionReport& report) {
    auto it = m_orders.find(report.order_id);
    if (it == m_orders.end()) {
        spdlog::error("Received execution report for unknown order ID: {}", report.order_id);
        return;
    }
    Order& order = it->second;
    order.status = report.new_status;
    double old_total_value = order.avg_fill_price * order.filled_quantity;
    double new_fill_value = report.fill_price * report.fill_quantity;
    order.filled_quantity += report.fill_quantity;
    if (order.filled_quantity > 0) {
        order.avg_fill_price = (old_total_value + new_fill_value) / order.filled_quantity;
    }
    double& position = m_positions[order.symbol];
    if (order.side == Side::BUY) {
        position += report.fill_quantity;
    } else {
        position -= report.fill_quantity;
    }
    spdlog::info("Updated order {}. New status: {}. New position for {}: {}",
                 order.order_id, status_to_string(order.status), order.symbol, position);
}

Order OrderManager::get_order(uint64_t order_id) const {
    auto it = m_orders.find(order_id);
    if (it != m_orders.end()) {
        return it->second;
    }
    return Order{};
}

double OrderManager::get_position(const std::string& symbol) const {
    auto it = m_positions.find(symbol);
    if (it != m_positions.end()) {
        return it->second;
    }
    return 0.0;
}

}
