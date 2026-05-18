#include <iostream>

#include "storage/StorageTier.hpp"
#include "storage/StorageManager.hpp"
#include "db/ObjectMetadata.hpp"
#include "db/DatabaseManager.hpp"
#include "core/ObjectManager.hpp"
#include "core/AccessManager.hpp"
#include "migration/MigrationEngine.hpp"
#include "migration/AutoTieringEngine.hpp"
#include "core/BackgroundTieringDaemon.hpp"
#include "core/PolicyManager.hpp"
#include "monitoring/MonitoringEngine.hpp"
#include "api/ApiServer.hpp"
#include "utils/Logger.hpp"

using namespace autotierx;

int main() {

    Logger::info(
    "AutoTierX system initialized"
);

Logger::warning(
    "Warm tier usage approaching threshold"
);

Logger::error(
    "Cold tier latency detected"
);

    /*
    =========================================
    STORAGE MANAGER
    =========================================
    */

    StorageManager manager;

    PolicyManager policyManager;
    policyManager.loadPolicies(
        "/home/vedant/autotierx/config/policies.json"
    );

    StorageTier hot(
        TierType::HOT,
        "HOT",
        "/home/vedant/hot-storage",
        100,
        true
    );

    StorageTier warm(
        TierType::WARM,
        "WARM",
        "/media/vedant/warm",
        59,
        true
    );

    StorageTier cold(
        TierType::COLD,
        "COLD",
        "/media/vedant/cold",
        14,
        true
    );

    StorageTier archive(
        TierType::ARCHIVE,
        "ARCHIVE",
        "/home/vedant/archive-storage",
        500,
        false
    );

    manager.addTier(hot);
    manager.addTier(warm);
    manager.addTier(cold);
    manager.addTier(archive);

    /*
    =========================================
    PRINT ALL TIERS
    =========================================
    */

    std::cout << std::endl;
    std::cout << "===== STORAGE TIERS ====="
              << std::endl;

    manager.printAllTiers();

    /*
    =========================================
    SAMPLE METADATA OBJECT
    =========================================
    */

    ObjectMetadata file1(
        "OBJ001",
        "video.mp4",
        "/home/vedant/hot-storage/video.mp4",
        "HOT",
        104857600,
        12,
        "2026-05-11",
        "2026-05-11",
        "abc123checksum"
    );

    std::cout << std::endl;
    std::cout << "===== OBJECT METADATA ====="
              << std::endl;

    file1.printMetadata();

    /*
    =========================================
    OBJECT MANAGER
    =========================================
    */

    ObjectManager objectManager;

    objectManager.ingestObject(
        "/home/vedant/autotierx/input/sample.txt",
        "/home/vedant/hot-storage"
    );

    std::cout << std::endl;
    std::cout << "===== STORED OBJECTS ====="
              << std::endl;

    objectManager.printAllObjects();

    /*
    =========================================
    OBJECT ACCESS ENGINE
    =========================================
    */

    AccessManager accessManager;

    auto& objects = objectManager.getObjects();

    accessManager.accessObject(objects[0]);

    /*
    =========================================
    DATABASE MANAGER
    =========================================
    */

    DatabaseManager dbManager;

    dbManager.connect(
        "/home/vedant/autotierx/metadata/metadata.db"
    );

    std::cout << std::endl;
    std::cout << "===== DATABASE CONTENT ====="
              << std::endl;

    dbManager.printAllMetadata();

    /*
    =========================================
    MIGRATION ENGINE
    =========================================
    */

    MigrationEngine migrationEngine;

    bool migOk = migrationEngine.migrateObject(
        objectManager.getObjects()[0],
        "/media/vedant/warm"
    );

    if (!migOk) {
        std::cout << "Initial migration failed or skipped." << std::endl;
    }

    /*
    =========================================
    AUTO TIERING ENGINE
    =========================================
    */

    AutoTieringEngine autoEngine;

    autoEngine.evaluateAndMigrate(
        objectManager.getObjects()
    );

    /*
    =========================================
    MONITORING ENGINE
    =========================================
    */

    MonitoringEngine monitoringEngine;
    monitoringEngine.monitorStorage(manager);

    /*
    =========================================
    FINISHED
    =========================================
    */

    std::cout << std::endl;
    std::cout
        << "===== AUTOTIERX EXECUTION COMPLETE ====="
        << std::endl;

    /*
    =========================================
    API SERVER
    =========================================
    */

    AutoTieringEngine autoTieringEngine;

    BackgroundTieringDaemon daemon(
        dbManager,
        migrationEngine,
        manager,
        policyManager
    );

    daemon.start();

    ApiServer apiServer;
    apiServer.start(
        manager,
        objectManager
    );

    return 0;
}