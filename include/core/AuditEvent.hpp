#pragma once

#include <string>

namespace autotierx {

struct AuditEvent {

    std::string timestamp;

    std::string eventType;

    std::string objectName;

    std::string details;
};

}