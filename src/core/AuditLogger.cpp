#include "../../include/core/AuditLogger.hpp"

#include <filesystem>
#include <fstream>

namespace autotierx {

void AuditLogger::logEvent(
    const AuditEvent& event
) {
    events.push_back(event);

    std::filesystem::create_directories(
        "/home/vedant/autotierx/logs"
    );

    std::ofstream logfile(
        "/home/vedant/autotierx/logs/audit.log",
        std::ios::app
    );

    if (logfile.is_open()) {
        logfile
            << event.timestamp
            << " | "
            << event.eventType
            << " | "
            << event.objectName
            << " | "
            << event.details
            << "\n";
    }
}

void AuditLogger::addEvent(
    const AuditEvent& event
) {
    events.push_back(event);
}

std::vector<AuditEvent>
AuditLogger::getEvents() const {

    return events;
}

}