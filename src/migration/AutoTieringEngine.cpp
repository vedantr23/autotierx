#include "../../include/migration/AutoTieringEngine.hpp"
#include "../../include/db/DatabaseManager.hpp"

#include "../../include/migration/MigrationEngine.hpp"
#include "../../include/utils/TierUtils.hpp"
#include "../../include/utils/Logger.hpp"

#include <iostream>
#include <filesystem>

namespace autotierx {

std::string AutoTieringEngine::determineTargetTier(
    const ObjectMetadata& object
) {

    /*
    =========================================
    HOT
    =========================================
    */

    if (object.getAccessCount() >= 10) {

        return "HOT";
    }

    /*
    =========================================
    WARM
    =========================================
    */

    if (object.getAccessCount() >= 5) {

        return "WARM";
    }

    /*
    =========================================
    COLD
    =========================================
    */

    if (object.getAccessCount() >= 1) {

        return "COLD";
    }

    /*
    =========================================
    ARCHIVE
    =========================================
    */

    return "ARCHIVE";
}

void AutoTieringEngine::evaluateAndMigrate(
    std::vector<ObjectMetadata>& objects
) {

    std::cout << std::endl;

    std::cout
        << "===== AUTO TIERING ENGINE ====="
        << std::endl;

    for (auto& object : objects) {

        std::string targetTier =
            determineTargetTier(object);

        /*
        =========================================
        SHOW RECOMMENDATION
        =========================================
        */

        std::cout << std::endl;

        std::cout
            << "Object: "
            << object.getFilename()
            << std::endl;

        std::cout
            << "Current Tier: "
            << object.getTier()
            << std::endl;

        std::cout
            << "Recommended Tier: "
            << targetTier
            << std::endl;

        /*
        =========================================
        MIGRATION DECISION
        =========================================
        */

        if (targetTier != object.getTier()) {

            std::cout
                << "[AUTO MIGRATION STARTED]"
                << std::endl;

            std::string sourcePath =
                object.getPath();

            std::string filename =
                object.getFilename();

            std::string targetPath =
                TierUtils::getTierPath(targetTier);

            MigrationEngine migrationEngine;

            migrationEngine.migrateObject(
                object,
                targetPath
            );

            object.setTier(targetTier);

            object.setPath(
                targetPath + "/" + filename
            );

            DatabaseManager dbManager;

            dbManager.connect(
                "/home/vedant/autotierx/metadata/metadata.db"
            );

            dbManager.updateObjectTier(
                object.getObjectId(),
                targetTier
            );

            dbManager.updateObjectPath(
                object.getObjectId(),
                targetPath + "/" + filename
            );

            Logger::info(
                "Auto-tier migration completed for " +
                filename
            );

        } else {

            std::cout
                << "[NO MIGRATION NEEDED]"
                << std::endl;
        }
    }
}

}