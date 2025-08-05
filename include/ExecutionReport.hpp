#pragma once

#include "types.hpp"
#include <string>
#include <chrono>

namespace TradingEngine {

    struct ExecutionReport {
        uint64_t report_id;

        uint64_t order_id;

        std::string symbol;

        OrderStatus new_status;

        double fill_quantity;

        double fill_price;

        std::chrono::system_clock::time_point execution_timestamp;

        ExecutionReport() : report_id(0), order_id(0), new_status(OrderStatus::NEW),
                            fill_quantity(0.0), fill_price(0.0),
                            execution_timestamp(std::chrono::system_clock::now()) {}
    };

} // namespace TradingEngine
