#pragma once

#include "I_MarketDataHandler.hpp"
#include "EWrapper.h"
#include "EClientSocket.h"
#include "EReaderOSSignal.h"
#include "EReader.h"
#include <memory>
#include <thread>

namespace TradingEngine {

class IBKRMarketDataHandler : public I_MarketDataHandler, public EWrapper {
public:
    IBKRMarketDataHandler(EngineCore& engine_core, const std::string& host, int port, int client_id);
    ~IBKRMarketDataHandler() override;

    void connect() override;
    void disconnect() override;

private:
    void process_messages();

    void nextValidId(OrderId orderId) override;
    void error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) override;
    void tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attrib) override;
    void tickSize(TickerId tickerId, TickType field, Decimal size) override;
    void openOrder(OrderId orderId, const Contract&, const ::Order&, const OrderState&) override;
    void pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) override;
    void completedOrder(const Contract& contract, const ::Order& order, const OrderState& orderState) override;
    void tickOptionComputation(TickerId, TickType, int, double, double, double, double, double, double, double, double) override;
    void tickGeneric(TickerId, TickType, double) override;
    void tickString(TickerId, TickType, const std::string&) override;
    void tickEFP(TickerId, TickType, double, const std::string&, double, int, const std::string&, double, double) override;
    void orderStatus(OrderId, const std::string&, Decimal, Decimal, double, int, int, double, int, const std::string&, double) override;
    void openOrderEnd() override;
    void winError(const std::string&, int) override;
    void connectionClosed() override;
    void updateAccountValue(const std::string&, const std::string&, const std::string&, const std::string&) override;
    void updatePortfolio(const Contract&, Decimal, double, double, double, double, double, const std::string&) override;
    void updateAccountTime(const std::string&) override;
    void accountDownloadEnd(const std::string&) override;
    void contractDetails(int, const ContractDetails&) override;
    void bondContractDetails(int, const ContractDetails&) override;
    void contractDetailsEnd(int) override;
    void execDetails(int, const Contract&, const Execution&) override;
    void execDetailsEnd(int) override;
    void updateMktDepth(TickerId, int, int, int, double, Decimal) override;
    void updateMktDepthL2(TickerId, int, const std::string&, int, int, double, Decimal, bool) override;
    void updateNewsBulletin(int, int, const std::string&, const std::string&) override;
    void managedAccounts(const std::string&) override;
    void receiveFA(faDataType, const std::string&) override;
    void historicalData(TickerId, const Bar&) override;
    void historicalDataEnd(int, const std::string&, const std::string&) override;
    void scannerParameters(const std::string&) override;
    void scannerData(int, int, const ContractDetails&, const std::string&, const std::string&, const std::string&, const std::string&) override;
    void scannerDataEnd(int) override;
    void realtimeBar(TickerId, long, double, double, double, double, Decimal, Decimal, int) override;
    void fundamentalData(TickerId, const std::string&) override;
    void deltaNeutralValidation(int, const DeltaNeutralContract&) override;
    void tickSnapshotEnd(int) override;
    void marketDataType(TickerId, int) override;
    void commissionReport(const CommissionReport&) override;
    void position(const std::string&, const Contract&, Decimal, double) override;
    void positionEnd() override;
    void accountSummary(int, const std::string&, const std::string&, const std::string&, const std::string&) override;
    void accountSummaryEnd(int) override;
    void verifyMessageAPI(const std::string&) override;
    void verifyCompleted(bool, const std::string&) override;
    void verifyAndAuthMessageAPI(const std::string&, const std::string&) override;
    void verifyAndAuthCompleted(bool, const std::string&) override;
    void displayGroupList(int, const std::string&) override;
    void displayGroupUpdated(int, const std::string&) override;
    void connectAck() override;
    void positionMulti(int, const std::string&, const std::string&, const Contract&, Decimal, double) override;
    void positionMultiEnd(int) override;
    void accountUpdateMulti(int, const std::string&, const std::string&, const std::string&, const std::string&, const std::string&) override;
    void accountUpdateMultiEnd(int) override;
    void securityDefinitionOptionalParameter(int, const std::string&, int, const std::string&, const std::string&, const std::set<std::string>&, const std::set<double>&) override;
    void securityDefinitionOptionalParameterEnd(int) override;
    void softDollarTiers(int, const std::vector<SoftDollarTier>&) override;
    void familyCodes(const std::vector<FamilyCode>&) override;
    void symbolSamples(int, const std::vector<ContractDescription>&) override;
    void mktDepthExchanges(const std::vector<DepthMktDataDescription>&) override;
    void tickByTickAllLast(int, int, time_t, double, Decimal, const TickAttribLast&, const std::string&, const std::string&) override;
    void tickByTickBidAsk(int, time_t, double, double, Decimal, Decimal, const TickAttribBidAsk&) override;
    void tickByTickMidPoint(int, time_t, double) override;
    void pnl(int, double, double, double) override;
    void historicalNews(int, const std::string&, const std::string&, const std::string&, const std::string&) override;
    void historicalNewsEnd(int, bool) override;
    void newsArticle(int, int, const std::string&) override;
    void newsProviders(const std::vector<NewsProvider>&) override;
    void historicalDataUpdate(TickerId, const Bar&) override;
    void rerouteMktDataReq(int, int, const std::string&) override;
    void rerouteMktDepthReq(int, int, const std::string&) override;
    void marketRule(int, const std::vector<PriceIncrement>&) override;
    void headTimestamp(int, const std::string&) override;
    void historicalTicks(int, const std::vector<HistoricalTick>&, bool) override;
    void historicalTicksBidAsk(int, const std::vector<HistoricalTickBidAsk>&, bool) override;
    void historicalTicksLast(int, const std::vector<HistoricalTickLast>&, bool) override;
    void completedOrdersEnd() override;
    void replaceFAEnd(int, const std::string&) override;
    void wshMetaData(int, const std::string&) override;
    void wshEventData(int, const std::string&) override;
    void historicalSchedule(int, const std::string&, const std::string&, const std::string&, const std::vector<HistoricalSession>&) override;
    void userInfo(int, const std::string&) override;
    void currentTime(long) override;
    void tickNews(int, time_t, const std::string&, const std::string&, const std::string&, const std::string&) override;
    void smartComponents(int, const SmartComponentsMap&) override;
    void tickReqParams(int, double, const std::string&, int) override;
    void histogramData(int, const HistogramDataVector&) override;
    void orderBound(long long, int, int) override;

private:
    std::unique_ptr<EClientSocket> m_client;
    EReaderOSSignal m_signal;
    std::unique_ptr<EReader> m_reader;
    std::thread m_reader_thread;

    std::string m_host;
    int m_port;
    int m_client_id;
    OrderId m_next_valid_id;
};

}
