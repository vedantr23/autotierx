#include "../../include/utils/Logger.hpp"

#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace autotierx {

void Logger::writeLog(
    const std::string& level,
    const std::string& message
) {

    std::ofstream logFile(
        "/home/vedant/autotierx/logs/system.log",
        std::ios::app
    );

    std::string timestamp = getCurrentTimestamp();

    logFile
        << "["
        << level
        << "] "
        << timestamp
        << " : "
        << message
        << std::endl;

    logFile.close();

    /*
    =====================================
    TERMINAL OUTPUT
    =====================================
    */

    std::cout
        << "["
        << level
        << "] "
        << timestamp
        << " : "
        << message
        << std::endl;
}

void Logger::info(
    const std::string& message
) {

    writeLog(
        "INFO",
        message
    );
}

void Logger::warning(
    const std::string& message
) {

    writeLog(
        "WARNING",
        message
    );
}

void Logger::error(
    const std::string& message
) {

    writeLog(
        "ERROR",
        message
    );
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

}