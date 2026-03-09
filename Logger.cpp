#include "Logger.h"

std::mutex log_mutex;

void Logger::log(LogLevel level, const std::string& message) {
    // Get current timestamp
    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp),
        "%Y-%m-%d %H:%M:%S", timeinfo);

    // Log to console
    std::lock_guard<std::mutex> lock(log_mutex); // Lock before logging

    #ifndef NDEBUG
    std::cout << "[" << timestamp << "] "
        << levelToString(level) << ": " << message
        << std::endl;
    #endif
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}