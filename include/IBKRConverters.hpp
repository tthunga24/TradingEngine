#pragma once

#include "Order.hpp"
#include "Order.h"
#include "Contract.h"

inline ::Contract convert_to_ibkr_contract(const TradingEngine::Order& order) {
    ::Contract contract;
    contract.symbol = order.symbol;
    contract.secType = "STK";
    contract.exchange = "SMART";
    contract.currency = "USD";
    return contract;
}

inline ::Order convert_to_ibkr_order(const TradingEngine::Order& order) {
    ::Order ibkr_order;
    ibkr_order.action = (order.side == TradingEngine::Side::BUY) ? "BUY" : "SELL";
    ibkr_order.totalQuantity = order.quantity;
    ibkr_order.orderType = (order.order_type == TradingEngine::OrderType::MARKET) ? "MKT" : "LMT";
    if (order.order_type == TradingEngine::OrderType::LIMIT) {
        ibkr_order.lmtPrice = order.price;
    }
    return ibkr_order;
}
