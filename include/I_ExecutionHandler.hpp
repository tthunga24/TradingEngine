#pragma once
#include "Order.hpp"

namespace TradingEngine {
    class I_ExecutionHandler {
    public:
        virtual ~I_ExecutionHandler() = default;
        virtual void place_order(Order& order) = 0;
    };
}
