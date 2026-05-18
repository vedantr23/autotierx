#pragma once

#include "../storage/StorageManager.hpp"
#include "../core/ObjectManager.hpp"
#include "../core/AuditLogger.hpp"

namespace autotierx {

class ApiServer {

public:

    void start(
        StorageManager& manager,
        ObjectManager& objectManager,
        AuditLogger& auditLogger
    );
};

}