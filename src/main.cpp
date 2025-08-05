#include "LogHandler.hpp" 
#include "ConfigHandler.hpp"
#include "EngineCore.hpp"
#include "OrderManager.hpp"
#include "MockMarketDataHandler.hpp" 

#include <csignal> 

using namespace TradingEngine;

EngineCore* g_engine_core_ptr = nullptr;

void signal_handler(int signal) {
    spdlog::warn("Exit signal recived, shutting down engine.", signal);
    if (g_engine_core_ptr) {
        g_engine_core_ptr->stop();
    }
}

int main(int argc, char* argv[]) {
    LogHandler::initialize();
    if (!ConfigHandler::initialize()) {
        spdlog::critical("Failed to initialize configuration. Shutting down.");
        return 1;
    }
    
    std::signal(SIGINT, signal_handler);

    spdlog::info("--- Trading Engine Starting ---");

    OrderManager order_manager;
    EngineCore engine_core(order_manager);
    g_engine_core_ptr = &engine_core; 

    MockMarketDataHandler market_data_handler(engine_core, "data/ticks.csv");
    market_data_handler.connect(); 

    engine_core.run();

    market_data_handler.disconnect();

    spdlog::info("--- Trading Engine Shutdown Complete ---");

    return 0;
}
