#include "LogHandler.hpp"
#include "TimeUtils.hpp"
#include "ConfigHandler.hpp"
#include "EngineCore.hpp"
#include "OrderManager.hpp"
#include "IBKRExecutionHandler.hpp"
#include "IBKRGatewayClient.hpp"
#include "MockMarketDataHandler.hpp"
#include "I_MarketDataHandler.hpp"
#include <csignal>
#include <memory>

using namespace TradingEngine;

std::unique_ptr<EngineCore> g_engine_core_ptr = nullptr;

void signal_handler(int signal) {
    if (g_engine_core_ptr) {
        g_engine_core_ptr->stop();
    }
}

int main(int argc, char* argv[]) {
    LogHandler::initialize();
    if (!ConfigHandler::initialize()) {
        return 1;
    }
    std::signal(SIGINT, signal_handler);
    spdlog::info("--- Trading Engine Starting ---");
    auto order_manager = std::make_unique<OrderManager>();
    g_engine_core_ptr = std::make_unique<EngineCore>(*order_manager, ConfigHandler::get_scripting_publish_endpoint(), ConfigHandler::get_scripting_subscribe_endpoint());
    auto execution_handler = std::make_unique<IBKRExecutionHandler>(g_engine_core_ptr.get());
    std::unique_ptr<I_MarketDataHandler> data_handler;
    std::string mode = ConfigHandler::get_engine_mode();
    g_engine_core_ptr->set_mode(mode);
    if (mode == "mock") {
	std::cout << g_engine_core_ptr.get() << std::endl;
        data_handler = std::make_unique<MockMarketDataHandler>(g_engine_core_ptr.get(), "data/ticks.csv");
        spdlog::info("Operating in MOCK mode.");
    } else {
        data_handler = std::make_unique<IBKRGatewayClient>(g_engine_core_ptr.get(), "127.0.0.1", 4002, 1);
        spdlog::info("Operating in LIVE/PAPER mode.");
    }
    g_engine_core_ptr->set_execution_handler(execution_handler.get());
    g_engine_core_ptr->set_market_data_handler(data_handler.get());
    if (mode != "mock") {
        g_engine_core_ptr->set_gateway_client(dynamic_cast<IBKRGatewayClient*>(data_handler.get()));
    }
    g_engine_core_ptr->startup();
    data_handler->connect();

    if (mode != "mock") {

	spdlog::info("Waiting for market open...");
	while (!is_market_open_now()) {
   	// if (!g_engine_core_ptr->m_is_running) { 
     	//   spdlog::warn("Shutdown requested");
       	// data_handler->disconnect();
      	//  return 0;
   	// }
    	spdlog::info("Market is closed. Waiting for 5 seconds");
    	std::this_thread::sleep_for(std::chrono::seconds(5));

    }
    
    spdlog::info("Market is open! Starting event loop.");
}

    g_engine_core_ptr->run();
    data_handler->disconnect();
    spdlog::info("--- Trading Engine Shutdown Complete ---");
    return 0;
}
