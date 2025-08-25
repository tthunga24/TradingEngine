#include "IBKRGatewayClient.hpp"
#include "LogHandler.hpp"
#include "EngineCore.hpp"
#include "Event.hpp"
#include "Order.h"
#include "Contract.h"
#include "Decimal.h"

namespace TradingEngine {

    IBKRGatewayClient::IBKRGatewayClient(EngineCore& engine_core, const std::string& host, int port, int client_id)
        : m_engine_core(engine_core), 
          m_host(host), m_port(port), m_client_id(client_id), 
          m_client(std::make_unique<EClientSocket>(this, &m_signal)), 
          m_signal(1000),
          m_is_connected(false) {
    }
    
    IBKRGatewayClient::~IBKRGatewayClient() {
        disconnect();
    }

    bool IBKRGatewayClient::connect() {
        spdlog::info("Connecting to IBKR TWS/Gateway at {}:{} with client ID {}...", m_host, m_port, m_client_id);
        m_is_connected = m_client->eConnect(m_host.c_str(), m_port, m_client_id);
        
        if (m_is_connected) {
            spdlog::info("Successfully initiated IBKR connection.");
            m_reader = std::make_unique<EReader>(m_client.get(), &m_signal);
            m_reader_thread = std::thread(&IBKRGatewayClient::process_messages, this);
        } else {
            spdlog::error("Failed to initiate IBKR connection.");
        }
        return m_is_connected;
    }

    void IBKRGatewayClient::disconnect() {
        if (!m_is_connected) return;
        m_is_connected = false;
        m_client->eDisconnect();
        if(m_reader_thread.joinable()) {
            m_reader_thread.join();
        }
        spdlog::info("Disconnected from IBKR.");
    }

    void IBKRGatewayClient::request_market_data(TickerId tickerId, const Contract& contract) {
        spdlog::info("Requesting market data for tickerId {}", tickerId);
        m_client->reqMktData(tickerId, contract, "", false, false, TagValueListSPtr());
    }

    void IBKRGatewayClient::place_order(OrderId orderId, const Contract& contract, const ::Order& order) {
        spdlog::info("Placing order with id {}", orderId);
        m_client->placeOrder(orderId, contract, order);
    }
    
    void IBKRGatewayClient::process_messages() {
        while (m_is_connected) {
             m_signal.waitForSignal();
             if (m_is_connected) { // Re-check after waking up
                m_reader->processMsgs();
             }
        }
    }

    // --- EWrapper Callback Implementations ---
    
    void IBKRGatewayClient::nextValidId(OrderId orderId) {
        spdlog::info("Connection to IBKR established. Next Valid Order ID: {}", orderId);
        m_next_valid_id = orderId;
    }

    void IBKRGatewayClient::tickPrice(TickerId tickerId, TickType, double price, const TickAttrib&) {
        spdlog::info("IBKR Tick Price. TickerId: {}, Price: {}", tickerId, price);
        // Here you would post a Tick event to the EngineCore
    }
    
    void IBKRGatewayClient::tickSize(TickerId tickerId, TickType, Decimal size) {
        spdlog::info("IBKR Tick Size. TickerId: {}, Size: {}", tickerId, DecimalFunctions::decimalToString(size));
    }
    
    void IBKRGatewayClient::error(int id, int errorCode, const std::string& errorString, const std::string&) {
        spdlog::error("IBKR Error. ID: {}, Code: {}, Message: {}", id, errorCode, errorString);
    }

    // --- All other required empty overrides ---
    void IBKRGatewayClient::openOrder(OrderId, const ::Contract&, const ::Order&, const ::OrderState&) {}
    void IBKRGatewayClient::pnlSingle(int, Decimal, double, double, double, double) {}
    void IBKRGatewayClient::completedOrder(const ::Contract&, const ::Order&, const ::OrderState&) {}
    void IBKRGatewayClient::tickOptionComputation(TickerId, TickType, int, double, double, double, double, double, double, double, double) {}
    void IBKRGatewayClient::tickGeneric(TickerId, TickType, double) {}
    void IBKRGatewayClient::tickString(TickerId, TickType, const std::string&) {}
    void IBKRGatewayClient::tickEFP(TickerId, TickType, double, const std::string&, double, int, const std::string&, double, double) {}
    void IBKRGatewayClient::orderStatus(OrderId, const std::string&, Decimal, Decimal, double, int, int, double, int, const std::string&, double) {}
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
    void IBKRGatewayClient::execDetails(int, const Contract&, const Execution&) {}
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

} // namespace TradingEngine
