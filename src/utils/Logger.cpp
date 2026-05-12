#include "../../include/utils/Logger.hpp"

#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>

namespace autotierx {

void Logger::writeLog(
    const std::string& level,
    const std::string& message
) {

    std::ofstream logFile(
        "/home/vedant/autotierx/logs/system.log",
        std::ios::app
    );

    auto now =
        std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

    logFile
        << "["
        << level
        << "] "
        << std::ctime(&now)
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

}