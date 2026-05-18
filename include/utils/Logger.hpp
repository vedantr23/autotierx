#pragma once

#include <string>

namespace autotierx {

class Logger {

public:

    static void info(
        const std::string& message
    );

    static void warning(
        const std::string& message
    );

    static void error(
        const std::string& message
    );

    static std::string getCurrentTimestamp();

private:

    static void writeLog(
        const std::string& level,
        const std::string& message
    );
};

}