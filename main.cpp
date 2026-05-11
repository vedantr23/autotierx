#include <iostream>

#include "storage/StorageTier.hpp"
#include "storage/StorageManager.hpp"
#include "db/ObjectMetadata.hpp"
#include "db/DatabaseManager.hpp"
#include "core/ObjectManager.hpp"
#include "migration/MigrationEngine.hpp"
#include "migration/AutoTieringEngine.hpp"

using namespace autotierx;

int main() {

    /*
    =========================================
    STORAGE MANAGER
    =========================================
    */

    StorageManager manager;

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

    migrationEngine.migrateObject(
        "/home/vedant/hot-storage/sample.txt",
        "/media/vedant/warm"
    );

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
    FINISHED
    =========================================
    */

    std::cout << std::endl;
    std::cout
        << "===== AUTOTIERX EXECUTION COMPLETE ====="
        << std::endl;

    return 0;
}