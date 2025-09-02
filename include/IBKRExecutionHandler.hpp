#pragma once
#include "I_ExecutionHandler.hpp"

namespace TradingEngine {
class EngineCore;

class IBKRExecutionHandler : public I_ExecutionHandler {
public:
    explicit IBKRExecutionHandler(EngineCore* engine_core);
    void place_order(Order& order) override;
    void set_engine_core(EngineCore* engine_core);
private:
    EngineCore* m_engine_core;
};

}
