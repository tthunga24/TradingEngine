#pragma once 

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <vector>
#include <iostream> 

class LogHandler {
public:
    LogHandler(const LogHandler&) = delete;
    LogHandler& operator=(const LogHandler&) = delete;

    /**
     * @brief Initializes the loggers. Must be called once at the start of the application.
     * @param log_file_path The path to the file where logs will be saved.
     */
    static void initialize(const std::string& log_file_path = "engine.log") {
        if (get_instance().m_initialized) {
            return; // Already initialized
        }

        try {
            std::vector<spdlog::sink_ptr> sinks;
            
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace); // Log all levels to console
            sinks.push_back(console_sink);

            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);
            file_sink->set_level(spdlog::level::info); // Only log info and above to the file
            sinks.push_back(file_sink);

            get_instance().m_logger = std::make_shared<spdlog::logger>("engine_logger", begin(sinks), end(sinks));
            
            get_instance().m_logger->set_level(spdlog::level::trace);
            spdlog::register_logger(get_instance().m_logger);
            
            spdlog::set_default_logger(get_instance().m_logger);

            get_instance().m_initialized = true;
            spdlog::info("LogHandler initialized successfully.");

        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

private:
    LogHandler() : m_initialized(false) {}

    static LogHandler& get_instance() {
        static LogHandler instance;
        return instance;
    }

    bool m_initialized;
    std::shared_ptr<spdlog::logger> m_logger;
};
