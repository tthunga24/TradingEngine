#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <stdexcept> // Added for std::runtime_error
#include "LogHandler.hpp" 

class ConfigHandler {
public:
    ConfigHandler(const ConfigHandler&) = delete;
    ConfigHandler& operator=(const ConfigHandler&) = delete;

    static bool initialize(const std::string& config_path = "config/config.json");

    static std::string get_engine_mode();
    static std::string get_log_file_path();
    static int get_max_order_size();
    static double get_max_position_value();
    static std::vector<std::string> get_market_data_subscriptions();
    
    // New methods for Scripting Interface
    static std::string get_scripting_publish_endpoint();
    static std::string get_scripting_subscribe_endpoint();

private:
    ConfigHandler() = default;

    static ConfigHandler& get_instance();

    template<typename T>
    T get_value(const std::string& key, const T& default_value) const {
        try {
            std::string pointer_path = "/" + key;
            std::replace(pointer_path.begin(), pointer_path.end(), '.', '/');
            nlohmann::json::json_pointer ptr(pointer_path);

            return m_config_json.at(ptr).get<T>();
        } catch (const nlohmann::json::out_of_range& e) {
            spdlog::warn("Config key '{}' not found. Using default value.", key);
            return default_value;
        } catch (const nlohmann::json::type_error& e) {
            spdlog::error("Config key '{}' has wrong type. Using default value. Error: {}", key, e.what());
            return default_value;
        }
    }

    template<typename T>
    T get_required_value(const std::string& key) const {
        try {
            std::string pointer_path = "/" + key;
            std::replace(pointer_path.begin(), pointer_path.end(), '.', '/');
            nlohmann::json::json_pointer ptr(pointer_path);

            return m_config_json.at(ptr).get<T>();
        } catch (const nlohmann::json::out_of_range& e) {
            spdlog::critical("Required config key '{}' not found.", key);
            throw std::runtime_error("Required config key not found: " + key);
        } catch (const nlohmann::json::type_error& e) {
            spdlog::critical("Required config key '{}' has wrong type.", key);
            throw std::runtime_error("Required config key has wrong type: " + key);
        }
    }

    nlohmann::json m_config_json;
    bool m_initialized = false;
};
