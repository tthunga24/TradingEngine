#include "ConfigHandler.hpp"
#include <fstream>

bool ConfigHandler::initialize(const std::string& config_path) {
    auto& instance = get_instance();
    if (instance.m_initialized) {
        spdlog::warn("ConfigHandler is already initialized.");
        return true;
    }
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        spdlog::critical("Failed to open configuration file: {}", config_path);
        return false;
    }
    try {
        instance.m_config_json = nlohmann::json::parse(config_file);
        instance.m_initialized = true;
        spdlog::info("ConfigHandler initialized successfully from {}", config_path);
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        spdlog::critical("Failed to parse config file: {}. Error: {}", config_path, e.what());
        return false;
    }
}

ConfigHandler& ConfigHandler::get_instance() {
    static ConfigHandler instance;
    return instance;
}

std::string ConfigHandler::get_engine_mode() {
    return get_instance().get_value<std::string>("engine_settings.mode", "mock");
}

std::string ConfigHandler::get_log_file_path() {
    return get_instance().get_value<std::string>("engine_settings.log_file_path", "logs/engine.log");
}

int ConfigHandler::get_max_order_size() {
    return get_instance().get_value<int>("risk_management.max_order_size", 100);
}

double ConfigHandler::get_max_position_value() {
    return get_instance().get_value<double>("risk_management.max_position_value_usd", 10000.0);
}

std::vector<std::string> ConfigHandler::get_market_data_subscriptions() {
    auto& instance = get_instance();
    if (instance.m_config_json.contains("market_data_subscriptions")) {
        return instance.m_config_json.at("market_data_subscriptions").get<std::vector<std::string>>();
    }
    return {};
}

std::string ConfigHandler::get_scripting_publish_endpoint() {
    return get_instance().get_required_value<std::string>("scripting.publish_endpoint");
}

std::string ConfigHandler::get_scripting_subscribe_endpoint() {
    return get_instance().get_required_value<std::string>("scripting.subscribe_endpoint");
}
