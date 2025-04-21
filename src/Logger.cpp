#include "../include/Logger.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>
#include <filesystem>

static std::unique_ptr<Logger> instance;
namespace fs = std::filesystem;

void Logger::init(const std::string& filename) {
    if (!instance) {
        instance = std::make_unique<Logger>();

        fs::create_directory("logs");  // ✅ create logs/ if missing
        const std::string fullpath = "logs/" + filename;

        instance->file.open(fullpath, std::ios::app);
    }
}

Logger& Logger::get() {
    if (!instance) {
        throw std::runtime_error("Logger not initialized. Call Logger::init(filename) first.");
    }
    return *instance;
}

Logger::~Logger() {
    if (file.is_open()) file.close();
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    std::string entry = "[" + timestamp() + "] [" + levelToString(level) + "] " + message;
    //std::cout << entry << std::endl;
    if (file.is_open()) file << entry << std::endl;
}

void Logger::info(const std::string& msg)  { log(LogLevel::INFO, msg); }
void Logger::warn(const std::string& msg)  { log(LogLevel::WARN, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::ERROR, msg); }

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

std::string Logger::timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};

    localtime_r(&t, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
