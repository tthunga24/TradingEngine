#pragma once

#include "I_MarketDataHandler.hpp"
#include <string>
#include <thread>
#include <atomic>

namespace TradingEngine {

class MockMarketDataHandler : public I_MarketDataHandler {
public:
    MockMarketDataHandler(EngineCore* engine_core, const std::string& csv_path);
    ~MockMarketDataHandler() override;

    void connect() override;
    void disconnect() override;
    void set_engine_core(EngineCore* engine_core);

private:
    void process_data_feed();
    EngineCore* m_engine_core;
    std::string m_csv_path;
    std::thread m_data_thread;
    std::atomic<bool> m_is_running;
};

}
