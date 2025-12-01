#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Standalone ZMQ Client to test Historical Data Request
// Compilation (example): 
// g++ -std=c++17 tests/TestHistoricalDataZMQ.cpp -o test_history_zmq -lzmq

int main() {
    zmq::context_t context(1);

    std::cout << "[CLIENT] Initializing ZMQ Context..." << std::endl;

    // 1. Setup Subscriber (To receive data FROM Engine)
    // Connect to the port defined in config.json "publish_endpoint" (default 5555)
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5555");
    
    // Subscribe specifically to "HISTORY" messages. 
    // The engine publishes topics like "HISTORY.AAPL", so "HISTORY" catches all.
    subscriber.set(zmq::sockopt::subscribe, "HISTORY");
    std::cout << "[CLIENT] Connected to Data Sub (5555) and subscribed to 'HISTORY'" << std::endl;

    // 2. Setup Publisher (To send commands TO Engine)
    // Connect to the port defined in config.json "subscribe_endpoint" (default 5556)
    zmq::socket_t publisher(context, ZMQ_PUB);
    publisher.connect("tcp://localhost:5556");
    std::cout << "[CLIENT] Connected to Command Pub (5556)" << std::endl;

    // IMPORTANT: Allow time for the connection to establish (Slow Joiner)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 3. Prepare the Historical Data Request
    nlohmann::json req_json;
    req_json["symbol"] = "AAPL";
    req_json["end_date"] = "";          // Empty string = "Now"
    req_json["duration"] = "1 W";       // 1 Week of data
    req_json["bar_size"] = "1 day";     // Daily bars

    std::string command_topic = "REQUEST_HISTORY";
    std::string payload = req_json.dump();

    // 4. Send Request
    std::cout << "[CLIENT] Sending REQUEST_HISTORY for AAPL..." << std::endl;
    // Send as multipart message: [Topic] + [Payload]
    publisher.send(zmq::buffer(command_topic), zmq::send_flags::sndmore);
    publisher.send(zmq::buffer(payload), zmq::send_flags::none);

    // 5. Listen for Responses
    std::cout << "[CLIENT] Waiting for incoming bars..." << std::endl;
    
    int bars_received = 0;
    while (true) {
        zmq::message_t topic_msg;
        auto res = subscriber.recv(topic_msg, zmq::recv_flags::none);
        
        if (res.has_value()) {
            std::string topic = topic_msg.to_string();
            
            zmq::message_t payload_msg;
            subscriber.recv(payload_msg, zmq::recv_flags::none);
            std::string data_str = payload_msg.to_string();

            // Check if this is the history data we asked for
            if (topic.find("HISTORY") != std::string::npos) {
                try {
                    auto data = nlohmann::json::parse(data_str);
                    
                    std::cout << "[RECV] " << topic << " | "
                              << "Date: " << data["time"] << " | "
                              << "O: " << data["open"] << " | "
                              << "H: " << data["high"] << " | "
                              << "L: " << data["low"] << " | "
                              << "C: " << data["close"] << " | "
                              << "Vol: " << data["volume"] 
                              << std::endl;
                    
                    bars_received++;
                } catch (const std::exception& e) {
                    std::cerr << "[ERROR] Failed to parse JSON: " << e.what() << std::endl;
                }
            }
        }
        
        // Break after receiving some bars so the test finishes
        if (bars_received >= 5) {
            std::cout << "[CLIENT] Received 5 bars. Test Complete." << std::endl;
            break;
        }
    }

    return 0;
}
