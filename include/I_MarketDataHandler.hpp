#pragma once

namespace TradingEngine {
class EngineCore;
}

namespace TradingEngine {

class I_MarketDataHandler {
public:
    virtual ~I_MarketDataHandler() = default;

    virtual void connect() = 0;

    virtual void disconnect() = 0;

protected:
    explicit I_MarketDataHandler(EngineCore* engine_core)
        : m_engine_core(engine_core) {}

    EngineCore* m_engine_core;
};

}
