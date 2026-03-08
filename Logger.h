#ifndef LOGGER_H_
#define LOGGER_H_


#include <string>
#include <ctime>
#include <iostream>
#include <mutex>

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

namespace Logger {
    void log(LogLevel level, const std::string& message);
    std::string levelToString(LogLevel level);
}

#endif