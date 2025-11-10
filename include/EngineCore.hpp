#pragma once

#include "ThreadSafeQueue.hpp"
#include "Event.hpp"
#include "OrderManager.hpp"
#include "ScriptingInterface.hpp"
#include "I_MarketDataHandler.hpp"
#include <atomic>
#include <thread>

namespace TradingEngine {
class OrderManager;
class I_MarketDataHandler;
class IBKRExecutionHandler;
class IBKRGatewayClient;
}

namespace TradingEngine {

class EngineCore {
public:
    EngineCore(
        OrderManager& order_manager);
    void set_market_data_handler(I_MarketDataHandler* md_handler);
    void set_execution_handler(IBKRExecutionHandler* exec_handler);
    void set_gateway_client(IBKRGatewayClient* gateway_client);
    void startup();
    void run();
    bool is_running() const;
    void stop();
    void post_event(Event event); 
    void set_mode(std::string mode);
    std::string get_mode();
    void start_data_feed();
    
private:
    I_MarketDataHandler* m_market_data_handler;
    IBKRExecutionHandler* m_execution_handler;
    IBKRGatewayClient* m_gateway_client;
    void process_events();
    void handle_tick_event(const Tick& tick);
    void handle_order_request_event(Order& order);
    void handle_send_new_order_event(Order& order);
    void handle_execution_report_event(const ExecutionReport& report);

    std::atomic<bool> m_is_running;
    ThreadSafeQueue<Event> m_event_queue;

    OrderManager& m_order_manager;
    ScriptingInterface m_scripting_interface;
    std::string m_mode;
};

}
