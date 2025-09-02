#include "IBKRGatewayClient.hpp"
#include "LogHandler.hpp"
#include "EngineCore.hpp"
#include "Event.hpp"
#include "Order.h"
#include "Contract.h"
#include "Decimal.h"
#include "Execution.h"

namespace TradingEngine {

IBKRGatewayClient::IBKRGatewayClient(EngineCore* engine_core, const std::string& host, int port, int client_id)
    : I_MarketDataHandler(engine_core),
      m_engine_core(engine_core),
      m_host(host), m_port(port), m_client_id(client_id),
      m_client(std::make_unique<EClientSocket>(this, &m_signal)),
      m_signal(1000),
      m_next_ticker_id(2000),
      m_is_connected(false) {}

IBKRGatewayClient::~IBKRGatewayClient() {
    disconnect();
}

void IBKRGatewayClient::set_engine_core(EngineCore* engine_core) {
    m_engine_core = engine_core;
}

void IBKRGatewayClient::connect() {
    spdlog::info("Connecting to IBKR TWS/Gateway at {}:{}...", m_host, m_port);
    m_is_connected = m_client->eConnect(m_host.c_str(), m_port, m_client_id);
    if (!m_is_connected) {
        spdlog::error("Failed to initiate IBKR connection.");
    }
    m_reader = std::make_unique<EReader>(m_client.get(), &m_signal);
    m_reader_thread = std::thread(&IBKRGatewayClient::process_messages, this);
    m_reader->start();
    auto future = m_connection_promise.get_future();
    spdlog::info("Waiting for connection confirmation from server...");
    if (future.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
        spdlog::error("IBKR connection timeout.");
        disconnect();
    }
    if (!m_is_connection_acknowledged) {
        disconnect();
    }
    spdlog::info("IBKR connection established and acknowledged.");
}

void IBKRGatewayClient::subscribe_to_market_data(const std::string& topic) {
    size_t last_dot = topic.find_last_of('.');
    if (last_dot == std::string::npos) {
        spdlog::error("Invalid subscription topic format: {}", topic);
        return;
    }
    std::string symbol = topic.substr(last_dot + 1);
    Contract contract;
    contract.symbol = symbol;
    contract.secType = "STK";
    contract.exchange = "SMART";
    contract.currency = "USD";
    TickerId new_id = m_next_ticker_id++;
    request_market_data(new_id, contract);
}

void IBKRGatewayClient::nextValidId(OrderId orderId) {
    spdlog::info("Connection to IBKR established. Next Valid Order ID: {}", orderId);
    m_next_valid_id = orderId;
    m_is_connection_acknowledged = true;
    m_connection_promise.set_value();
    spdlog::info("Setting market data type to Delayed (3).");
    m_client->reqMarketDataType(3);
}

void IBKRGatewayClient::disconnect() {
    if (!m_is_connected && !m_client->isConnected())
        return;
    m_is_connected = false;
    m_client->eDisconnect();
    m_signal.issueSignal();
    if (m_reader_thread.joinable()) {
        m_reader_thread.join();
    }
    spdlog::info("Disconnected from IBKR.");
}

void IBKRGatewayClient::request_market_data(TickerId tickerId, const Contract& contract) {
    m_ticker_id_to_symbol[tickerId] = contract.symbol;
    spdlog::info("Requesting market data for {} (TickerId {})", contract.symbol, tickerId);
    m_client->reqMktData(tickerId, contract, "", false, false, TagValueListSPtr());
}

void IBKRGatewayClient::place_order(OrderId orderId, const Contract& contract, const ::Order& order) {
    spdlog::info("Placing order with id {}", orderId);
    m_client->placeOrder(orderId, contract, order);
}

void IBKRGatewayClient::process_messages() {
    spdlog::info("Reader thread started.");
    while (m_is_connected && m_client->isConnected()) {
        m_reader->processMsgs();
    }
    if (m_is_connected) {
        spdlog::warn("Connection to IBKR was lost.");
        disconnect();
    }
    spdlog::info("Reader thread finished.");
}

void IBKRGatewayClient::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib&) {
    if (price <= 0.0)
        return;
    if (field == TickType::LAST || field == TickType::DELAYED_LAST) {
        if (m_ticker_id_to_symbol.find(tickerId) == m_ticker_id_to_symbol.end()) {
            spdlog::warn("Received tick for unknown TickerId: {}", tickerId);
            return;
        }
        Tick tick;
        tick.symbol = m_ticker_id_to_symbol[tickerId];
        tick.price = price;
        tick.timestamp = std::chrono::system_clock::now();
        Event tick_event;
        tick_event.type = EventType::TICK;
        tick_event.data = tick;
        if (m_engine_core) {
            m_engine_core->post_event(tick_event);
        }
    }
}

void IBKRGatewayClient::tickSize(TickerId tickerId, TickType field, Decimal size) {
    if (field == TickType::LAST_SIZE || field == TickType::DELAYED_LAST_SIZE) {
        if (m_ticker_id_to_symbol.count(tickerId)) {
            spdlog::info("Tick Size for {}: {}", m_ticker_id_to_symbol[tickerId], DecimalFunctions::decimalToString(size));
        }
    }
}

void IBKRGatewayClient::error(int id, int errorCode, const std::string& errorString, const std::string&) {
    spdlog::error("IBKR Error. ID: {}, Code: {}, Message: {}", id, errorCode, errorString);
    if (errorCode == 502 || errorCode == 504 || errorCode == 522) {
        m_is_connection_acknowledged = false;
        m_connection_promise.set_value();
    }
}

void IBKRGatewayClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
                                    Decimal remaining, double avgFillPrice, int permId,
                                    int parentId, double lastFillPrice, int clientId,
                                    const std::string& whyHeld, double mktCapPrice) {

    spdlog::info("Order Status. Id: {}, Status: {}, Filled: {}, Remaining: {}, AvgFillPrice: {}",
                 orderId, status, DecimalFunctions::decimalToString(filled),
                 DecimalFunctions::decimalToString(remaining), avgFillPrice);

    ExecutionReport report;
    report.order_id = orderId;
    report.fill_quantity = 0; // orderStatus doesn't always have fill details, execDetails does
    report.fill_price = avgFillPrice;

    Event report_event;
    report_event.type = EventType::EXECUTION_REPORT;
    report_event.data = report;
    if (m_engine_core) {
        m_engine_core->post_event(report_event);
    }
}

void IBKRGatewayClient::execDetails(int reqId, const Contract& contract, const Execution& execution) {
    spdlog::info("Execution Details. OrderId: {}, Symbol: {}, Side: {}, Quantity: {}, Price: {}", 
                 execution.orderId, contract.symbol, execution.side, 
                 DecimalFunctions::decimalToString(execution.shares), execution.price);
    
}

void IBKRGatewayClient::openOrder(OrderId, const ::Contract&, const ::Order&, const ::OrderState&) {}
void IBKRGatewayClient::pnlSingle(int, Decimal, double, double, double, double) {}
void IBKRGatewayClient::completedOrder(const ::Contract&, const ::Order&, const ::OrderState&) {}
void IBKRGatewayClient::tickOptionComputation(TickerId, TickType, int, double, double, double, double, double, double, double, double) {}
void IBKRGatewayClient::tickGeneric(TickerId, TickType, double) {}
void IBKRGatewayClient::tickString(TickerId, TickType, const std::string&) {}
void IBKRGatewayClient::tickEFP(TickerId, TickType, double, const std::string&, double, int, const std::string&, double, double) {}
void IBKRGatewayClient::openOrderEnd() {}
void IBKRGatewayClient::winError(const std::string& str, int lastError) { spdlog::error("IBKR WinError. Error: {}, Message: {}", lastError, str); }
void IBKRGatewayClient::connectionClosed() { spdlog::warn("IBKR connection closed."); m_is_connected = false; m_signal.issueSignal(); }
void IBKRGatewayClient::updateAccountValue(const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::updatePortfolio(const Contract&, Decimal, double, double, double, double, double, const std::string&) {}
void IBKRGatewayClient::updateAccountTime(const std::string&) {}
void IBKRGatewayClient::accountDownloadEnd(const std::string&) {}
void IBKRGatewayClient::contractDetails(int, const ContractDetails&) {}
void IBKRGatewayClient::bondContractDetails(int, const ContractDetails&) {}
void IBKRGatewayClient::contractDetailsEnd(int) {}
void IBKRGatewayClient::execDetailsEnd(int) {}
void IBKRGatewayClient::updateMktDepth(TickerId, int, int, int, double, Decimal) {}
void IBKRGatewayClient::updateMktDepthL2(TickerId, int, const std::string&, int, int, double, Decimal, bool) {}
void IBKRGatewayClient::updateNewsBulletin(int, int, const std::string&, const std::string&) {}
void IBKRGatewayClient::managedAccounts(const std::string&) {}
void IBKRGatewayClient::receiveFA(faDataType, const std::string&) {}
void IBKRGatewayClient::historicalData(TickerId, const Bar&) {}
void IBKRGatewayClient::scannerParameters(const std::string&) {}
void IBKRGatewayClient::scannerData(int, int, const ContractDetails&, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::scannerDataEnd(int) {}
void IBKRGatewayClient::realtimeBar(TickerId, long, double, double, double, double, Decimal, Decimal, int) {}
void IBKRGatewayClient::currentTime(long) {}
void IBKRGatewayClient::fundamentalData(TickerId, const std::string&) {}
void IBKRGatewayClient::deltaNeutralValidation(int, const DeltaNeutralContract&) {}
void IBKRGatewayClient::tickSnapshotEnd(int) {}
void IBKRGatewayClient::marketDataType(TickerId, int) {}
void IBKRGatewayClient::commissionReport(const CommissionReport&) {}
void IBKRGatewayClient::position(const std::string&, const Contract&, Decimal, double) {}
void IBKRGatewayClient::positionEnd() {}
void IBKRGatewayClient::accountSummary(int, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::accountSummaryEnd(int) {}
void IBKRGatewayClient::verifyMessageAPI(const std::string&) {}
void IBKRGatewayClient::verifyCompleted(bool, const std::string&) {}
void IBKRGatewayClient::verifyAndAuthMessageAPI(const std::string&, const std::string&) {}
void IBKRGatewayClient::verifyAndAuthCompleted(bool, const std::string&) {}
void IBKRGatewayClient::displayGroupList(int, const std::string&) {}
void IBKRGatewayClient::displayGroupUpdated(int, const std::string&) {}
void IBKRGatewayClient::connectAck() {}
void IBKRGatewayClient::positionMulti(int, const std::string&, const std::string&, const Contract&, Decimal, double) {}
void IBKRGatewayClient::positionMultiEnd(int) {}
void IBKRGatewayClient::accountUpdateMulti(int, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::accountUpdateMultiEnd(int) {}
void IBKRGatewayClient::securityDefinitionOptionalParameter(int, const std::string&, int, const std::string&, const std::string&, const std::set<std::string>&, const std::set<double>&) {}
void IBKRGatewayClient::securityDefinitionOptionalParameterEnd(int) {}
void IBKRGatewayClient::softDollarTiers(int, const std::vector<SoftDollarTier>&) {}
void IBKRGatewayClient::familyCodes(const std::vector<FamilyCode>&) {}
void IBKRGatewayClient::symbolSamples(int, const std::vector<ContractDescription>&) {}
void IBKRGatewayClient::historicalDataEnd(int, const std::string&, const std::string&) {}
void IBKRGatewayClient::mktDepthExchanges(const std::vector<DepthMktDataDescription>&) {}
void IBKRGatewayClient::tickNews(int, time_t, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::smartComponents(int, const SmartComponentsMap&) {}
void IBKRGatewayClient::tickReqParams(int, double, const std::string&, int) {}
void IBKRGatewayClient::newsProviders(const std::vector<NewsProvider>&) {}
void IBKRGatewayClient::newsArticle(int, int, const std::string&) {}
void IBKRGatewayClient::historicalNews(int, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRGatewayClient::historicalNewsEnd(int, bool) {}
void IBKRGatewayClient::headTimestamp(int, const std::string&) {}
void IBKRGatewayClient::histogramData(int, const HistogramDataVector&) {}
void IBKRGatewayClient::historicalDataUpdate(TickerId, const Bar&) {}
void IBKRGatewayClient::rerouteMktDataReq(int, int, const std::string&) {}
void IBKRGatewayClient::rerouteMktDepthReq(int, int, const std::string&) {}
void IBKRGatewayClient::marketRule(int, const std::vector<PriceIncrement>&) {}
void IBKRGatewayClient::pnl(int, double, double, double) {}
void IBKRGatewayClient::historicalTicks(int, const std::vector<HistoricalTick>&, bool) {}
void IBKRGatewayClient::historicalTicksBidAsk(int, const std::vector<HistoricalTickBidAsk>&, bool) {}
void IBKRGatewayClient::historicalTicksLast(int, const std::vector<HistoricalTickLast>&, bool) {}
void IBKRGatewayClient::tickByTickAllLast(int, int, time_t, double, Decimal, const TickAttribLast&, const std::string&, const std::string&) {}
void IBKRGatewayClient::tickByTickBidAsk(int, time_t, double, double, Decimal, Decimal, const TickAttribBidAsk&) {}
void IBKRGatewayClient::tickByTickMidPoint(int, time_t, double) {}
void IBKRGatewayClient::orderBound(long long, int, int) {}
void IBKRGatewayClient::completedOrdersEnd() {}
void IBKRGatewayClient::replaceFAEnd(int, const std::string&) {}
void IBKRGatewayClient::wshMetaData(int, const std::string&) {}
void IBKRGatewayClient::wshEventData(int, const std::string&) {}
void IBKRGatewayClient::historicalSchedule(int, const std::string&, const std::string&, const std::string&, const std::vector<HistoricalSession>&) {}
void IBKRGatewayClient::userInfo(int, const std::string&) {}

}
