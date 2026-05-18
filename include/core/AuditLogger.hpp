#pragma once

#include "AuditEvent.hpp"

#include <vector>

namespace autotierx {

class AuditLogger {

private:

    std::vector<AuditEvent> events;

public:

    void logEvent(
        const AuditEvent& event
    );

    void addEvent(
        const AuditEvent& event
    );

    std::vector<AuditEvent>
    getEvents() const;
};

}