#pragma once

#include "../db/ObjectMetadata.hpp"
#include "../db/DatabaseManager.hpp"
#include "../migration/MigrationEngine.hpp"
#include "../storage/StorageManager.hpp"
#include "PolicyManager.hpp"

#include <thread>
#include <atomic>
#include <vector>

namespace autotierx {

class BackgroundTieringDaemon {

private:

    DatabaseManager& dbManager;

    MigrationEngine& migrationEngine;

    StorageManager& storageManager;

    PolicyManager& policyManager;

    std::atomic<bool> running;

    std::thread daemonThread;

public:

    BackgroundTieringDaemon(
        DatabaseManager& db,
        MigrationEngine& migration,
        StorageManager& storage,
        PolicyManager& policy
    );

    void start();

    void stop();

    void processObjects();
};

}
