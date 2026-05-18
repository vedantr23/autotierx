#include "../../include/migration/MigrationEngine.hpp"

#include <filesystem>
#include "../../include/db/DatabaseManager.hpp"
#include "../../include/api/WebsocketBroadcaster.hpp"
#include "../../include/utils/Logger.hpp"
#include <iostream>

namespace fs = std::filesystem;

namespace autotierx {

bool MigrationEngine::migrateObject(
    ObjectMetadata& object,
    const std::string& destinationTier
) {

    std::string sourcePath = object.getPath();
    std::string sourceTier = object.getTier();
    std::string filename;

    try {

        fs::path source(sourcePath);

        if (!fs::exists(source)) {
            Logger::warning("Migration failed: source object not found: " + sourcePath);
            DatabaseManager dbManager;
            dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
            dbManager.insertAuditLog(
                "migrate",
                "",
                sourceTier,
                destinationTier,
                "failure",
                "Source object not found"
            );
            std::cout
                << "Source object not found."
                << std::endl;

            return false;
        }

        filename = source.filename().string();

        // Ensure destination tier path exists
        if (!fs::exists(destinationTier)) {

            Logger::warning("Migration failed: destination tier unavailable: " + destinationTier);
            DatabaseManager dbManager;
            dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
            dbManager.insertAuditLog(
                "migrate",
                filename,
                sourceTier,
                destinationTier,
                "failure",
                "Destination tier path unavailable"
            );
            std::cout
                << "Destination tier path not available: "
                << destinationTier
                << std::endl;

            return false;
        }

        std::string destinationPath =
            destinationTier + "/" + filename;

        /*
        =====================================
        STEP 1 → COPY FILE
        =====================================
        */

        fs::copy_file(
            source,
            destinationPath,
            fs::copy_options::overwrite_existing
        );

        /*
        =====================================
        STEP 2 → VERIFY COPY
        =====================================
        */

        if (!fs::exists(destinationPath)) {

            std::cout
                << "Migration verification failed."
                << std::endl;

            return false;
        }

        /*
        =====================================
        STEP 3 → DELETE ORIGINAL
        =====================================
        */

        fs::remove(source);

        /*
        =====================================
        SUCCESS LOGS
        =====================================
        */

        std::cout << std::endl;

        std::cout
            << "[MIGRATION SUCCESS]"
            << std::endl;

        std::cout
            << "FROM: "
            << sourcePath
            << std::endl;

        std::cout
            << "TO: "
            << destinationPath
            << std::endl;

        Logger::info("Migration success: " + filename + " from " + sourceTier + " to " + destinationTier);

        DatabaseManager dbManager;

        dbManager.connect(

            "/home/vedant/autotierx/metadata/metadata.db"
        );

        dbManager.insertMigrationHistory(

            filename,

            sourceTier,

            destinationTier
        );
        dbManager.insertAuditLog(
            "migrate",
            filename,
            sourceTier,
            destinationTier,
            "success",
            "Migration completed"
        );

        broadcastUpdate("refresh");

        return true;

    } catch (const std::exception& e) {

        Logger::error(std::string("Migration error: ") + e.what());
        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        dbManager.insertAuditLog(
            "migrate",
            filename.empty() ? (fs::path(sourcePath).has_filename() ? fs::path(sourcePath).filename().string() : "") : filename,
            sourceTier,
            destinationTier,
            "failure",
            std::string("Migration error: ") + e.what()
        );
        std::cout
            << "Migration Error: "
            << e.what()
            << std::endl;

        return false;
    }
}

}