#pragma once

#include "../storage/StorageManager.hpp"

namespace autotierx {

class MonitoringEngine {

public:

    void monitorStorage(
        const StorageManager& manager
    );
};

}