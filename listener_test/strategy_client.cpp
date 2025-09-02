#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main() {
    // --- Setup Sockets ---
    zmq::context_t context(1);

    // Socket for RECEIVING market data from the engine
    zmq::socket_t data_socket(context, ZMQ_SUB);
    data_socket.connect("tcp://localhost:5555");
    std::cout << "[CLIENT] Connected to engine's data channel (5555)." << std::endl;

    // Socket for SENDING commands to the engine
    zmq::socket_t command_socket(context, ZMQ_PUB);
    command_socket.connect("tcp://localhost:5556");
    std::cout << "[CLIENT] Connected to engine's command channel (5556)." << std::endl;

    // Subscribe to all TICK messages
    data_socket.set(zmq::sockopt::subscribe, "TICK");

    // --- State for our simple strategy ---
    int tick_counter = 0;
    bool order_sent = false;

    // Allow ZMQ time to establish connections before sending
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "\n[CLIENT] --- Strategy is running ---" << std::endl;
    
    std::string command_topic = "SUBSCRIBE";
    nlohmann::json payload_json;
    payload_json["topic"] = "TICK.TSLA";
    std::string payload_str = payload_json.dump();
    
    // 2. Send the two-part message
    std::cout << "Sending subscription request for " << payload_json["topic"] << "..." << std::endl;
    command_socket.send(zmq::buffer(command_topic), zmq::send_flags::sndmore);
    command_socket.send(zmq::buffer(payload_str), zmq::send_flags::none); 
    while (true) {
        // --- Receive Market Data ---
        zmq::message_t topic_msg;
        data_socket.recv(topic_msg, zmq::recv_flags::none);

        zmq::message_t payload_msg;
        data_socket.recv(payload_msg, zmq::recv_flags::none);

        std::cout << "[CLIENT] Received on topic '" << topic_msg.to_string()
                  << "': " << payload_msg.to_string() << std::endl;

        tick_counter++;

        // --- Strategy Logic ---
        // After 5 ticks, send a BUY order, but only once.
        if (tick_counter % 3 == 0) {
            std::cout << "\n[CLIENT] *** Tick threshold reached. Sending CREATE_ORDER command... ***\n" << std::endl;

            // 1. Define the command topic
            std::string command_topic = "CREATE_ORDER";

            // 2. Construct the JSON payload according to the API contract
            nlohmann::json order_payload;
            order_payload["command"] = "CREATE_ORDER";
            order_payload["correlation_id"] = "strat-client-001";
            order_payload["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
            order_payload["payload"]["symbol"] = "AAPL";
            order_payload["payload"]["side"] = "BUY";
            order_payload["payload"]["order_type"] = "MARKET";
            order_payload["payload"]["quantity"] = 50;
            
            std::string payload_str = order_payload.dump();

            // 3. Send the command as a multi-part message
            command_socket.send(zmq::buffer(command_topic), zmq::send_flags::sndmore);
            command_socket.send(zmq::buffer(payload_str), zmq::send_flags::none);

            order_sent = true; // Set the flag so we don't send more orders
        }
    }

    return 0;
}

