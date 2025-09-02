#pragma once

#include <string>

namespace TradingEngine {

enum class Side {
    BUY,
    SELL
};

enum class OrderStatus {
    NEW,
    PENDING_NEW,
    CONFIRMED,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

enum class OrderType {
    MARKET,
    LIMIT
};

inline std::string side_to_string(Side side) {
    switch (side) {
        case Side::BUY:
            return "BUY";
        case Side::SELL:
            return "SELL";
        default:
            return "UNKNOWN";
    }
}

inline std::string status_to_string(OrderStatus status) {
    switch (status) {
        case OrderStatus::NEW:
            return "NEW";
        case OrderStatus::PENDING_NEW:
            return "PENDING_NEW";
        case OrderStatus::CONFIRMED:
            return "CONFIRMED";
        case OrderStatus::PARTIALLY_FILLED:
            return "PARTIALLY_FILLED";
        case OrderStatus::FILLED:
            return "FILLED";
        case OrderStatus::CANCELED:
            return "CANCELED";
        case OrderStatus::REJECTED:
            return "REJECTED";
        default:
            return "UNKNOWN";
    }
}

}
