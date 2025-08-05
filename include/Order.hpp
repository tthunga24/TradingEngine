#pragma once

#include "types.hpp"
#include <string>
#include <chrono>

namespace TradingEngine {

    struct Order {
        uint64_t order_id;

        std::string symbol;

        Side side;

        OrderType order_type;

        OrderStatus status;

        double quantity;

        double price;

        double filled_quantity;

        double avg_fill_price;

        std::chrono::system_clock::time_point creation_timestamp;

        Order() : order_id(0), side(Side::BUY), order_type(OrderType::MARKET),
                  status(OrderStatus::NEW), quantity(0.0), price(0.0),
                  filled_quantity(0.0), avg_fill_price(0.0),
                  creation_timestamp(std::chrono::system_clock::now()) {}
    };

} // namespace TradingEngine
