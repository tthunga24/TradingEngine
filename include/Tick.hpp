#pragma once

#include <string>
#include <chrono>

namespace TradingEngine {

struct Tick {
    std::string symbol;
    double price;
    uint64_t size;
    std::chrono::system_clock::time_point timestamp;
};

}
