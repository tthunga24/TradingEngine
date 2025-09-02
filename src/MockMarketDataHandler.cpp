#include "MockMarketDataHandler.hpp"
#include "EngineCore.hpp"
#include "LogHandler.hpp"
#include "Event.hpp"
#include "Tick.hpp"
#include <fstream>
#include <sstream>
#include <chrono>

namespace TradingEngine {

MockMarketDataHandler::MockMarketDataHandler(EngineCore* engine_core, const std::string& csv_path)
    : I_MarketDataHandler(engine_core), m_csv_path(csv_path), m_is_running(false) {}

MockMarketDataHandler::~MockMarketDataHandler() {
    if (m_is_running) {
        disconnect();
    }
}

void MockMarketDataHandler::set_engine_core(EngineCore* engine_core) {
    m_engine_core = engine_core;
}

void MockMarketDataHandler::connect() {
    if (m_is_running) {
        spdlog::warn("MockMarketDataHandler is already connected.");
        return;
    }
    m_is_running = true;
    m_data_thread = std::thread(&MockMarketDataHandler::process_data_feed, this);
    spdlog::info("MockMarketDataHandler connected and started processing {}", m_csv_path);
}

void MockMarketDataHandler::disconnect() {
    m_is_running = false;
    if (m_data_thread.joinable()) {
        m_data_thread.join();
    }
    spdlog::info("MockMarketDataHandler disconnected.");
}

void MockMarketDataHandler::process_data_feed() {
    std::ifstream data_file(m_csv_path);
    if (!data_file.is_open()) {
        spdlog::critical("Failed to open market data file: {}", m_csv_path);
        m_is_running = false;
        return;
    }
    std::string line;
    while (m_is_running && std::getline(data_file, line)) {
        std::stringstream ss(line);
        std::string symbol, price_str, size_str;
        if (std::getline(ss, symbol, ',') && std::getline(ss, price_str, ',') && std::getline(ss, size_str)) {
            try {
                Tick tick;
                tick.symbol = symbol;
                tick.price = std::stod(price_str);
                tick.size = std::stoull(size_str);
                tick.timestamp = std::chrono::system_clock::now();
                Event tick_event;
                tick_event.type = EventType::TICK;
                tick_event.data = tick;
                m_engine_core->post_event(tick_event);
            } catch (const std::invalid_argument& e) {
                spdlog::error("Could not parse line in CSV: {}", line);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

}
