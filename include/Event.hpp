#pragma once
#include <variant>
#include <string>
#include "Tick.hpp"
#include "Order.hpp"
#include "Bar.hpp"
#include "ExecutionReport.hpp" 

namespace TradingEngine {

struct HistoricalDataRequest {
    std::string symbol;
    std::string end_date;  // Format: "yyyymmdd HH:mm:ss" or "" for now
    std::string duration;  // e.g., "1 W", "1 M"
    std::string bar_size;  // e.g., "1 day", "1 hour"
};

enum class EventType {
    TICK,
    ORDER_REQUEST,
    SEND_NEW_ORDER,        
    EXECUTION_REPORT,
    NEXT_VALID_ID,
    SYSTEM_SHUTDOWN,
    SUBSCRIBE_REQUEST,
    HISTORICAL_DATA_REQUEST,
    HISTORICAL_DATA         
};

struct Event {
    EventType type;
    std::variant<
        std::monostate,      
        Tick,
        Order,
        long long,
        ExecutionReport,
        std::string,
        HistoricalDataRequest,
        Bar                    
    > data;
};

}
