#include "ScriptingInterface.hpp"
#include "EngineCore.hpp"
#include "LogHandler.hpp"
#include "Event.hpp"
#include "Order.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace TradingEngine {

ScriptingInterface::ScriptingInterface(EngineCore& engine_core, const std::string& data_pub_endpoint, const std::string& command_sub_endpoint)
    : m_engine_core(engine_core),
      m_context(1),
      m_data_publisher(m_context, ZMQ_PUB),
      m_command_subscriber(m_context, ZMQ_SUB),
      m_data_pub_endpoint(data_pub_endpoint),
      m_command_sub_endpoint(command_sub_endpoint),
      m_is_running(false) {}

ScriptingInterface::~ScriptingInterface() {
    if (m_is_running) {
        stop();
    }
}


void ScriptingInterface::start() {
    m_is_running = true;
    spdlog::info("ScriptingInterface starting...");
    m_data_publisher.bind(m_data_pub_endpoint);
    m_command_subscriber.bind(m_command_sub_endpoint);
    spdlog::info("Data publisher bound to {}", m_data_pub_endpoint);
    spdlog::info("Command subscriber bound to {}", m_command_sub_endpoint);
    m_command_subscriber.set(zmq::sockopt::subscribe, "");
    m_command_thread = std::thread(&ScriptingInterface::listen_for_commands, this);
}

void ScriptingInterface::stop() {
    m_is_running = false;
    if (m_command_thread.joinable()) {
        m_command_thread.join();
    }
    spdlog::info("ScriptingInterface stopped.");
}

void ScriptingInterface::publish_tick(const Tick& tick) {
    std::string topic = "TICK." + tick.symbol;
    nlohmann::json payload_json;
    payload_json["timestamp"] = std::to_string(tick.timestamp.time_since_epoch().count());
    payload_json["data"]["symbol"] = tick.symbol;
    payload_json["data"]["price"] = tick.price;
    payload_json["data"]["size"] = tick.size;
    std::string payload_str = payload_json.dump();
    m_data_publisher.send(zmq::buffer(topic), zmq::send_flags::sndmore);
    m_data_publisher.send(zmq::buffer(payload_str), zmq::send_flags::none);
}

// Implement the new publishing method
void ScriptingInterface::publish_historical_data(const Bar& bar) {
    std::string topic = "HISTORY." + bar.symbol;
    nlohmann::json payload_json;
    payload_json["symbol"] = bar.symbol;
    payload_json["time"] = bar.time;
    payload_json["open"] = bar.open;
    payload_json["high"] = bar.high;
    payload_json["low"] = bar.low;
    payload_json["close"] = bar.close;
    payload_json["volume"] = bar.volume;

    std::string payload_str = payload_json.dump();
    m_data_publisher.send(zmq::buffer(topic), zmq::send_flags::sndmore);
    m_data_publisher.send(zmq::buffer(payload_str), zmq::send_flags::none);
}

void ScriptingInterface::listen_for_commands() {
    bool is_mock_mode = (m_engine_core.get_mode() == "mock");

    while (m_is_running) {
        zmq::message_t topic_msg;
        auto result = m_command_subscriber.recv(topic_msg, zmq::recv_flags::dontwait);
        
        if (!result.has_value()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::string topic = topic_msg.to_string();

        if (is_mock_mode && topic == "MOCK") {
            spdlog::info("Start signal received, beginning data feed");
            m_engine_core.start_data_feed();
            continue; 
        }

        if (topic == "REQUEST_HISTORY") {
             zmq::message_t payload_msg;
             auto res = m_command_subscriber.recv(payload_msg, zmq::recv_flags::none);
             if(res.has_value()){
                 try {
                     auto json = nlohmann::json::parse(payload_msg.to_string());
                     
                     // We need to define a struct for this or use a temporary carrier
                     // Assuming you add 'HistoricalDataRequest' to Event.hpp or use a specific event
                     // For now, I will use a simple struct-like logic
                     
                     HistoricalDataRequest req;
                     req.symbol = json["symbol"];
                     req.end_date = json.value("end_date", "");
                     req.duration = json.value("duration", "1 W");
                     req.bar_size = json.value("bar_size", "1 day");

                     Event event;
                     event.type = EventType::HISTORICAL_DATA_REQUEST;
                     event.data = req; // Ensure Event variant supports this!
                     m_engine_core.post_event(event);
                     
                     spdlog::info("Received history request for {}", req.symbol);

                 } catch (const std::exception& e) {
                     spdlog::error("Failed to parse REQUEST_HISTORY: {}", e.what());
                 }
             }
        }
        else if (topic == "SUBSCRIBE") {
            zmq::message_t payload_msg;
            m_command_subscriber.recv(payload_msg, zmq::recv_flags::none);
            try {
                auto payload = nlohmann::json::parse(payload_msg.to_string());
                std::string data_topic = payload.at("topic");
                Event sub_event;
                sub_event.type = EventType::SUBSCRIBE_REQUEST;
                sub_event.data = data_topic;
                m_engine_core.post_event(sub_event);
                spdlog::info("Received SUBSCRIBE request for topic: {}", data_topic);
            } catch (const std::exception& e) {
                spdlog::error("Could not parse SUBSCRIBE payload: {}", e.what());
            }
        }
        else if (topic == "CREATE_ORDER") {
            zmq::message_t payload_msg;
            auto payload_result = m_command_subscriber.recv(payload_msg, zmq::recv_flags::none);
            if (!payload_result.has_value()) {
                spdlog::error("Received CREATE_ORDER topic without a payload.");
                continue;
            }
            try {
                std::string payload_str = payload_msg.to_string();
                spdlog::debug("Received CREATE_ORDER payload: {}", payload_str);
                auto json_data = nlohmann::json::parse(payload_str);
                auto payload = json_data.contains("payload") ? json_data["payload"] : json_data;

                Order order;
                order.symbol = payload["symbol"].get<std::string>();
                order.quantity = payload["quantity"].get<double>();
                
                std::string side_str = payload["side"].get<std::string>();
                if (side_str == "BUY") order.side = Side::BUY;
                else if (side_str == "SELL") order.side = Side::SELL;
                
                std::string type_str = payload["order_type"].get<std::string>();
                if (type_str == "MARKET") order.order_type = OrderType::MARKET;
                else if (type_str == "LIMIT") {
                    order.order_type = OrderType::LIMIT;
                    order.price = payload.value("limit_price", 0.0);
                }

                Event order_event;
                order_event.type = EventType::ORDER_REQUEST;
                order_event.data = order;
                m_engine_core.post_event(order_event);
                spdlog::info("Posted ORDER_REQUEST for {}", order.symbol);
            } catch (const nlohmann::json::exception& e) {
                spdlog::error("Failed to parse CREATE_ORDER: {}", e.what());
            }
        }
    }
}

}
