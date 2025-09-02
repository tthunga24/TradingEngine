#include "IBKRExecutionHandler.hpp"
#include "EngineCore.hpp"
#include "Event.hpp"

namespace TradingEngine {
    IBKRExecutionHandler::IBKRExecutionHandler(EngineCore* engine_core)
        : m_engine_core(engine_core) {}

    void IBKRExecutionHandler::set_engine_core(EngineCore* engine_core) {
        m_engine_core = engine_core;
    }

    void IBKRExecutionHandler::place_order(Order& order) {
        Event order_event;
        order_event.type = EventType::SEND_NEW_ORDER;
        order_event.data = order;
        if (m_engine_core) {
            m_engine_core->post_event(order_event);
        }
    }
}
