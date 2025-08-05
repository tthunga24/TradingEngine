#pragma once

#include "Tick.hpp"
#include "Order.hpp"
#include "ExecutionReport.hpp"
#include <variant>

namespace TradingEngine {

    // Defines all possible types of events that can occur in the system
    enum class EventType {
        TICK,                // A market data tick has arrived
        ORDER_REQUEST,       // A strategy wants to place a new order
        EXECUTION_REPORT,    // An update about an order from the broker
        SYSTEM_SHUTDOWN      // A signal to gracefully shut down the engine
    };

    /**
     * @brief A struct representing a single event in the engine's queue.
     * It uses std::variant for type-safe storage of different event data.
     */
    struct Event {
        // The type of this event
        EventType type;

        // The data payload for the event.
        // std::monostate is a placeholder for events that have no data.
        std::variant<
            std::monostate, 
            Tick, 
            Order, 
            ExecutionReport
        > data;
    };

} // namespace TradingEngine
