#pragma once
#include <string>

namespace TradingEngine {

struct Bar {
    std::string symbol;
    std::string time;
    double open;
    double high;
    double low;
    double close;
    long long volume;
};

}
