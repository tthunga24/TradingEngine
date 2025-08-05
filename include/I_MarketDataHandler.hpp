#pragma once

namespace TradingEngine {
    class EngineCore;
}

namespace TradingEngine {

    /**
     * @brief Abstract base class (Interface) for all market data handlers.
     * * This class defines the common contract that any component responsible for
     * fetching market data must adhere to. This design allows us to easily
     * swap between a mock handler (for testing) and real handlers (for paper
     * or live trading) without changing the core engine logic.
     */
    class I_MarketDataHandler {
    public:
        virtual ~I_MarketDataHandler() = default;


        /**
         * @brief Initiates the connection to the market data source.
         * This might involve opening a network socket or loading a file.
         */
        virtual void connect() = 0;

        /**
         * @brief Terminates the connection to the market data source.
         */
        virtual void disconnect() = 0;

    protected:
        explicit I_MarketDataHandler(EngineCore& engine_core)
            : m_engine_core(engine_core) {}

        EngineCore& m_engine_core;
    };

} // namespace TradingEngine
