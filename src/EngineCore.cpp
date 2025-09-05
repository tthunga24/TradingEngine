#include "EngineCore.hpp"
#include "LogHandler.hpp"
#include "ConfigHandler.hpp"
#include "IBKRExecutionHandler.hpp"
#include "IBKRGatewayClient.hpp"
#include "I_MarketDataHandler.hpp"
#include "IBKRConverters.hpp"

namespace TradingEngine {

EngineCore::EngineCore(OrderManager& order_manager)
    : m_is_running(false),
      m_order_manager(order_manager),
      m_market_data_handler(nullptr),
      m_execution_handler(nullptr),
      m_gateway_client(nullptr),
      m_scripting_interface(*this, "tcp://*:5555", "tcp://*:5556") {}

void EngineCore::set_market_data_handler(I_MarketDataHandler* md_handler) {
    m_market_data_handler = md_handler;
}

void EngineCore::set_execution_handler(IBKRExecutionHandler* exec_handler) {
    m_execution_handler = exec_handler;
}

void EngineCore::set_gateway_client(IBKRGatewayClient* gateway_client) {
    m_gateway_client = gateway_client;
}

bool EngineCore::is_running() const { return m_is_running; }

void EngineCore::startup() {
    m_scripting_interface.start();
}

void EngineCore::run() {
    m_is_running = true;
    spdlog::info("EngineCore event loop is starting...");
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
            
            case EventType::SYSTEM_SHUTDOWN:
                m_is_running = false;
                break;
            
            case EventType::SUBSCRIBE_REQUEST: {
                if (const auto* topic_ptr = std::get_if<std::string>(&event.data)) {
                    if (m_gateway_client) {
                        m_gateway_client->subscribe_to_market_data(*topic_ptr);
                    } else {
                        spdlog::warn("Received SUBSCRIBE_REQUEST but gateway client is not set.");
                    }
                }
                break;
            }

            case EventType::ORDER_REQUEST: {
                Order order = std::get<Order>(event.data);
                spdlog::info("EngineCore processing order request for {} {} {}", 
                     side_to_string(order.side), order.quantity, order.symbol);

                m_order_manager.add_new_order(order);

                ::Order ibkr_order = convert_to_ibkr_order(order);
                ::Contract ibkr_contract = convert_to_ibkr_contract(order);

                if (m_gateway_client) {
                    m_gateway_client->place_order(order.order_id, ibkr_contract, ibkr_order);
                    spdlog::info("Order {} sent to the gateway.", order.order_id);
                } else {
                    spdlog::warn("Gateway client is not available. Order not sent.");
                }
                break;
            }

            case EventType::EXECUTION_REPORT: {
                const auto& report = std::get<ExecutionReport>(event.data);
                m_order_manager.update_order_status(report);
                break;
            }
            
            case EventType::NEXT_VALID_ID: {
                if (const auto* order_id_ptr = std::get_if<long long>(&event.data)) {
                    m_order_manager.set_next_order_id(*order_id_ptr);
                }
                break;
            }
            default:
                spdlog::warn("Received unhandled event type.");
                break;
        }
    }
}

void EngineCore::handle_tick_event(const Tick& tick) {
    m_scripting_interface.publish_tick(tick);
}

} // namespace TradingEngine
