#pragma once

#include <string>
#include <fstream>
#include <mutex>

enum class LogLevel {
    INFO,
    WARN,
    ERROR
};

class Logger {
    std::ofstream file;
    std::mutex mutex;

    static std::string levelToString(LogLevel level);
    static std::string timestamp();

public:
    Logger() = default;   // ✅ now accessible
    ~Logger();

    static void init(const std::string& filename);  // call once
    static Logger& get();                           // use anywhere

    void log(LogLevel level, const std::string& message);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
};

// Macros for convenience
#define LOG_INFO(msg)   Logger::get().info(msg)
#define LOG_WARN(msg)   Logger::get().warn(msg)
#define LOG_ERROR(msg)  Logger::get().error(msg)
