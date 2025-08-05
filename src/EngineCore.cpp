#include "EngineCore.hpp"
#include "LogHandler.hpp"
#include "ConfigHandler.hpp"

namespace TradingEngine {

    EngineCore::EngineCore(OrderManager& order_manager) :
        m_is_running(false),
        m_order_manager(order_manager),
        m_scripting_interface(
            *this,
            "tcp://*:5555",
            "tcp://*:5556"
        )
    {
    }

    void EngineCore::run() {
        m_is_running = true;
        m_scripting_interface.start();
        spdlog::info("EngineCore is starting...");
        process_events();
        m_scripting_interface.stop();
        spdlog::info("EngineCore has stopped.");
    }

    void EngineCore::stop() {
        Event shutdown_event;
        shutdown_event.type = EventType::SYSTEM_SHUTDOWN;
        post_event(shutdown_event);
    }

    void EngineCore::post_event(Event event) {
        m_event_queue.push(std::move(event));
    }

    void EngineCore::process_events() {
        while (m_is_running) {
            Event event;
            m_event_queue.wait_and_pop(event);

            switch (event.type) {
                case EventType::TICK:
                    handle_tick_event(std::get<Tick>(event.data));
                    break;
                
                case EventType::ORDER_REQUEST:
                    handle_order_request_event(std::get<Order>(event.data));
                    break;

                case EventType::SYSTEM_SHUTDOWN:
                    spdlog::info("Shutdown event received. Terminating event loop.");
                    m_is_running = false;
                    break;
                
                default:
                    spdlog::warn("Received unhandled event type.");
                    break;
            }
        }
    }


    void EngineCore::handle_tick_event(const Tick& tick) {
        spdlog::debug("Passing Tick to ScriptingInterface: {} @ {}", tick.symbol, tick.price);
        m_scripting_interface.publish_tick(tick);
    }

    void EngineCore::handle_order_request_event(Order& order) {
        spdlog::info("Processing Order Request Event for {} {} {}", 
                     side_to_string(order.side), order.quantity, order.symbol);
        
        uint64_t order_id = m_order_manager.add_new_order(order);
        
        spdlog::info("Order successfully processed and assigned Engine ID: {}", order_id);
        
    }

} // namespace TradingEngine

