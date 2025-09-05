#pragma once

#include "Tick.hpp"
#include "Order.hpp"
#include "ExecutionReport.hpp"
#include <variant>

namespace TradingEngine {

enum class EventType {
    TICK,
    ORDER_REQUEST,
    SEND_NEW_ORDER,
    EXECUTION_REPORT,
    NEXT_VALID_ID,
    SYSTEM_SHUTDOWN,
    SUBSCRIBE_REQUEST
};

struct Event {
    EventType type;
    std::variant<
        std::monostate,
        Tick,
        Order,
	long long,
        ExecutionReport,
        std::string
        > data;
};

}
