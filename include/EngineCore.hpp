#pragma once

#include "ThreadSafeQueue.hpp"
#include "Event.hpp"
#include "OrderManager.hpp"
#include "ScriptingInterface.hpp"
#include <atomic>
#include <thread>

namespace TradingEngine {

    class EngineCore {
    public:
        EngineCore(OrderManager& order_manager);

        void run();

        void stop();

        void post_event(Event event);

    private:
        void process_events();

        void handle_tick_event(const Tick& tick);
        void handle_order_request_event(Order& order);

        std::atomic<bool> m_is_running;
	ScriptingInterface m_scripting_interface;
        ThreadSafeQueue<Event> m_event_queue;

        OrderManager& m_order_manager;
    };

} // namespace TradingEngine
