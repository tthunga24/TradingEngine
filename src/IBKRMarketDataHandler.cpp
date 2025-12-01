#include "IBKRMarketDataHandler.hpp"
#include "LogHandler.hpp"
#include "EngineCore.hpp"
#include "Event.hpp"

namespace TradingEngine {

IBKRMarketDataHandler::IBKRMarketDataHandler(EngineCore& engine_core, const std::string& host, int port, int client_id)
    : I_MarketDataHandler(&engine_core), m_host(host), m_port(port), m_client_id(client_id),
      m_client(std::make_unique<EClientSocket>(this, &m_signal)), m_signal(1000) {}

IBKRMarketDataHandler::~IBKRMarketDataHandler() {
    disconnect();
}

void IBKRMarketDataHandler::connect() {
    spdlog::info("Connecting to IBKR TWS/Gateway at {}:{} with client ID {}...", m_host, m_port, m_client_id);
    if (m_client->eConnect(m_host.c_str(), m_port, m_client_id)) {
        spdlog::info("Successfully initiated IBKR connection.");
        m_reader = std::make_unique<EReader>(m_client.get(), &m_signal);
        m_reader_thread = std::thread(&IBKRMarketDataHandler::process_messages, this);
    } else {
        spdlog::error("Failed to initiate IBKR connection.");
    }
}

void IBKRMarketDataHandler::disconnect() {
    m_client->eDisconnect();
    if(m_reader_thread.joinable()) {
        m_reader_thread.join();
    }
    spdlog::info("Disconnected from IBKR.");
}

void IBKRMarketDataHandler::process_messages() {
    while (m_client->isConnected()) {
        m_signal.waitForSignal();
        m_reader->processMsgs();
    }
}

void IBKRMarketDataHandler::tickPrice(TickerId tickerId, TickType, double price, const TickAttrib&) {
    spdlog::info("IBKR Tick Price. TickerId: {}, Price: {}", tickerId, price);
}

void IBKRMarketDataHandler::tickSize(TickerId tickerId, TickType, Decimal size) {
    spdlog::info("IBKR Tick Size. TickerId: {}, Size: {}", tickerId, DecimalFunctions::decimalToString(size));
}

void IBKRMarketDataHandler::nextValidId(OrderId orderId) {
    spdlog::info("Connection to IBKR established. Next Valid Order ID: {}", orderId);
    m_next_valid_id = orderId;
}

void IBKRMarketDataHandler::error(int id, int errorCode, const std::string& errorString, const std::string&) {
    spdlog::error("IBKR Error. ID: {}, Code: {}, Message: {}", id, errorCode, errorString);
}

void IBKRMarketDataHandler::historicalData(TickerId reqId, const ::Bar& bar) {
    TradingEngine::Bar engine_bar;
    engine_bar.time = bar.time;
    engine_bar.open = bar.open;
    engine_bar.high = bar.high;
    engine_bar.low = bar.low;
    engine_bar.close = bar.close;
    engine_bar.volume = bar.volume;
    if(m_reqId_to_symbol_map.count(reqId)){
         engine_bar.symbol = m_reqId_to_symbol_map[reqId]; 
    }

    TradingEngine::Event event;
    event.type = TradingEngine::EventType::HISTORICAL_DATA;
    event.data = engine_bar;
    m_engine_core->post_event(event);
}

void IBKRMarketDataHandler::historicalDataUpdate(TickerId reqId, const ::Bar& bar) {
}

void IBKRMarketDataHandler::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) { 
    spdlog::info("Finished receiving historical data for request {}", reqId);
}

void IBKRMarketDataHandler::openOrder(OrderId, const Contract&, const ::Order&, const OrderState&) {}
void IBKRMarketDataHandler::pnlSingle(int, Decimal, double, double, double, double) {}
void IBKRMarketDataHandler::completedOrder(const Contract&, const ::Order&, const OrderState&) {}
void IBKRMarketDataHandler::tickOptionComputation(TickerId, TickType, int, double, double, double, double, double, double, double, double) {}
void IBKRMarketDataHandler::tickGeneric(TickerId, TickType, double) {}
void IBKRMarketDataHandler::tickString(TickerId, TickType, const std::string&) {}
void IBKRMarketDataHandler::tickEFP(TickerId, TickType, double, const std::string&, double, int, const std::string&, double, double) {}
void IBKRMarketDataHandler::orderStatus(OrderId, const std::string&, Decimal, Decimal, double, int, int, double, int, const std::string&, double) {}
void IBKRMarketDataHandler::openOrderEnd() {}
void IBKRMarketDataHandler::winError(const std::string& str, int lastError) { spdlog::error("IBKR WinError. Error: {}, Message: {}", lastError, str); }
void IBKRMarketDataHandler::connectionClosed() { spdlog::warn("IBKR connection closed."); }
void IBKRMarketDataHandler::updateAccountValue(const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::updatePortfolio(const Contract&, Decimal, double, double, double, double, double, const std::string&) {}
void IBKRMarketDataHandler::updateAccountTime(const std::string&) {}
void IBKRMarketDataHandler::accountDownloadEnd(const std::string&) {}
void IBKRMarketDataHandler::contractDetails(int, const ContractDetails&) {}
void IBKRMarketDataHandler::bondContractDetails(int, const ContractDetails&) {}
void IBKRMarketDataHandler::contractDetailsEnd(int) {}
void IBKRMarketDataHandler::execDetails(int, const Contract&, const Execution&) {}
void IBKRMarketDataHandler::execDetailsEnd(int) {}
void IBKRMarketDataHandler::updateMktDepth(TickerId, int, int, int, double, Decimal) {}
void IBKRMarketDataHandler::updateMktDepthL2(TickerId, int, const std::string&, int, int, double, Decimal, bool) {}
void IBKRMarketDataHandler::updateNewsBulletin(int, int, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::managedAccounts(const std::string&) {}
void IBKRMarketDataHandler::receiveFA(faDataType, const std::string&) {}
void IBKRMarketDataHandler::scannerParameters(const std::string&) {}
void IBKRMarketDataHandler::scannerData(int, int, const ContractDetails&, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::scannerDataEnd(int) {}
void IBKRMarketDataHandler::realtimeBar(TickerId, long, double, double, double, double, Decimal, Decimal, int) {}
void IBKRMarketDataHandler::fundamentalData(TickerId, const std::string&) {}
void IBKRMarketDataHandler::deltaNeutralValidation(int, const DeltaNeutralContract&) {}
void IBKRMarketDataHandler::tickSnapshotEnd(int) {}
void IBKRMarketDataHandler::marketDataType(TickerId, int) {}
void IBKRMarketDataHandler::commissionReport(const CommissionReport&) {}
void IBKRMarketDataHandler::position(const std::string&, const Contract&, Decimal, double) {}
void IBKRMarketDataHandler::positionEnd() {}
void IBKRMarketDataHandler::accountSummary(int, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::accountSummaryEnd(int) {}
void IBKRMarketDataHandler::verifyMessageAPI(const std::string&) {}
void IBKRMarketDataHandler::verifyCompleted(bool, const std::string&) {}
void IBKRMarketDataHandler::verifyAndAuthMessageAPI(const std::string&, const std::string&) {}
void IBKRMarketDataHandler::verifyAndAuthCompleted(bool, const std::string&) {}
void IBKRMarketDataHandler::displayGroupList(int, const std::string&) {}
void IBKRMarketDataHandler::displayGroupUpdated(int, const std::string&) {}
void IBKRMarketDataHandler::connectAck() {}
void IBKRMarketDataHandler::positionMulti(int, const std::string&, const std::string&, const Contract&, Decimal, double) {}
void IBKRMarketDataHandler::positionMultiEnd(int) {}
void IBKRMarketDataHandler::accountUpdateMulti(int, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::accountUpdateMultiEnd(int) {}
void IBKRMarketDataHandler::securityDefinitionOptionalParameter(int, const std::string&, int, const std::string&, const std::string&, const std::set<std::string>&, const std::set<double>&) {}
void IBKRMarketDataHandler::securityDefinitionOptionalParameterEnd(int) {}
void IBKRMarketDataHandler::softDollarTiers(int, const std::vector<SoftDollarTier>&) {}
void IBKRMarketDataHandler::familyCodes(const std::vector<FamilyCode>&) {}
void IBKRMarketDataHandler::symbolSamples(int, const std::vector<ContractDescription>&) {}
void IBKRMarketDataHandler::mktDepthExchanges(const std::vector<DepthMktDataDescription>&) {}
void IBKRMarketDataHandler::tickByTickAllLast(int, int, time_t, double, Decimal, const TickAttribLast&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::tickByTickBidAsk(int, time_t, double, double, Decimal, Decimal, const TickAttribBidAsk&) {}
void IBKRMarketDataHandler::tickByTickMidPoint(int, time_t, double) {}
void IBKRMarketDataHandler::pnl(int, double, double, double) {}
void IBKRMarketDataHandler::historicalNews(int, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::historicalNewsEnd(int, bool) {}
void IBKRMarketDataHandler::newsArticle(int, int, const std::string&) {}
void IBKRMarketDataHandler::newsProviders(const std::vector<NewsProvider>&) {}
void IBKRMarketDataHandler::rerouteMktDataReq(int, int, const std::string&) {}
void IBKRMarketDataHandler::rerouteMktDepthReq(int, int, const std::string&) {}
void IBKRMarketDataHandler::marketRule(int, const std::vector<PriceIncrement>&) {}
void IBKRMarketDataHandler::headTimestamp(int, const std::string&) {}
void IBKRMarketDataHandler::historicalTicks(int, const std::vector<HistoricalTick>&, bool) {}
void IBKRMarketDataHandler::historicalTicksBidAsk(int, const std::vector<HistoricalTickBidAsk>&, bool) {}
void IBKRMarketDataHandler::historicalTicksLast(int, const std::vector<HistoricalTickLast>&, bool) {}
void IBKRMarketDataHandler::completedOrdersEnd() {}
void IBKRMarketDataHandler::replaceFAEnd(int, const std::string&) {}
void IBKRMarketDataHandler::wshMetaData(int, const std::string&) {}
void IBKRMarketDataHandler::wshEventData(int, const std::string&) {}
void IBKRMarketDataHandler::historicalSchedule(int, const std::string&, const std::string&, const std::string&, const std::vector<HistoricalSession>&) {}
void IBKRMarketDataHandler::userInfo(int, const std::string&) {}
void IBKRMarketDataHandler::currentTime(long) {}
void IBKRMarketDataHandler::tickNews(int, time_t, const std::string&, const std::string&, const std::string&, const std::string&) {}
void IBKRMarketDataHandler::smartComponents(int, const SmartComponentsMap&) {}
void IBKRMarketDataHandler::tickReqParams(int, double, const std::string&, int) {}
void IBKRMarketDataHandler::histogramData(int, const HistogramDataVector&) {}
void IBKRMarketDataHandler::orderBound(long long, int, int) {}

}
