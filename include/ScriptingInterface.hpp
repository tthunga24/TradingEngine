#pragma once

#include <zmq.hpp>
#include <thread>
#include <atomic>
#include "Tick.hpp"

namespace TradingEngine {
class EngineCore;
}

namespace TradingEngine {

class ScriptingInterface {
public:
    ScriptingInterface(EngineCore& engine_core, const std::string& data_pub_endpoint, const std::string& command_sub_endpoint);
    ~ScriptingInterface();

    void start();

    void stop();

    void publish_tick(const Tick& tick);

private:
    void listen_for_commands();

    EngineCore& m_engine_core;
    zmq::context_t m_context;
    zmq::socket_t m_data_publisher;
    zmq::socket_t m_command_subscriber;

    std::string m_data_pub_endpoint;
    std::string m_command_sub_endpoint;

    std::thread m_command_thread;
    std::atomic<bool> m_is_running;
};

}
