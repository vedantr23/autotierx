#include "../../include/core/BackgroundTieringDaemon.hpp"
#include "../../include/utils/TierUtils.hpp"
#include "../../include/utils/Logger.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace autotierx {

BackgroundTieringDaemon::BackgroundTieringDaemon(
    DatabaseManager& db,
    MigrationEngine& migration,
    StorageManager& storage,
    PolicyManager& policy
)
    : dbManager(db),
      migrationEngine(migration),
      storageManager(storage),
      policyManager(policy),
      running(false) {
}

void BackgroundTieringDaemon::start() {

    running = true;

    daemonThread = std::thread([this]() {

        while (running) {

            std::cout
                << "\n[DAEMON] Running background tiering..."
                << std::endl;

            processObjects();

            std::this_thread::sleep_for(
                std::chrono::seconds(15)
            );
        }
    });
}

void BackgroundTieringDaemon::stop() {

    running = false;

    if (daemonThread.joinable()) {
        daemonThread.join();
    }
}

void BackgroundTieringDaemon::processObjects() {

    storageManager.refreshTierHealth();

    for (const auto& tier : storageManager.getTiers()) {
        if (!tier.isAvailable()) {
            Logger::warning("Tier offline detected: " + tier.getName());
            dbManager.insertAuditLog(
                "warning",
                "",
                tier.getName(),
                "",
                "failure",
                tier.getName() + " storage unavailable"
            );
        }
    }

    dbManager.insertAuditLog(
        "daemon",
        "",
        "",
        "",
        "info",
        "Background tiering cycle started"
    );

    auto objects = dbManager.getAllObjects();

    auto parseLastAccess = [](const std::string& timestamp) {
        std::tm tm = {};
        std::istringstream ss(timestamp);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        if (ss.fail()) {
            ss.clear();
            ss.str(timestamp);
            ss >> std::get_time(&tm, "%a %b %d %H:%M:%S %Y");
            if (ss.fail()) {
                return std::time_t(0);
            }
        }
        return std::mktime(&tm);
    };

    auto now = std::chrono::system_clock::now();

    for (auto& object : objects) {

        std::cout
            << "[DAEMON] Checking: "
            << object.getFilename()
            << std::endl;

        std::time_t lastAccessTime =
            parseLastAccess(object.getLastAccessed());

        if (lastAccessTime == 0) {
            std::cout
                << "[DAEMON] Invalid last access timestamp for: "
                << object.getFilename()
                << std::endl;
            dbManager.insertAuditLog(
                "warning",
                object.getFilename(),
                object.getTier(),
                "",
                "failure",
                "Invalid last access timestamp"
            );
            continue;
        }

        auto lastAccess =
            std::chrono::system_clock::from_time_t(
                lastAccessTime
            );

        auto age =
            std::chrono::duration_cast<
                std::chrono::hours
            >(now - lastAccess).count() / 24;

        std::string targetTier;

        if (age >=
            policyManager.getArchiveDays() &&
            object.getTier() != "ARCHIVE") {

            std::cout
                << "[DAEMON] Aging object to ARCHIVE"
                << std::endl;

            targetTier = "ARCHIVE";

        } else if (age >=
                   policyManager.getColdDays() &&
                   object.getTier() != "COLD") {

            std::cout
                << "[DAEMON] Aging object to COLD"
                << std::endl;

            targetTier = "COLD";

        } else if (age >=
                   policyManager.getWarmDays() &&
                   object.getTier() != "WARM") {

            std::cout
                << "[DAEMON] Aging object to WARM"
                << std::endl;

            targetTier = "WARM";
        }

        if (targetTier.empty()) {
            continue;
        }

        std::string targetPath =
            TierUtils::getTierPath(targetTier);

        bool success = migrationEngine.migrateObject(
            object,
            targetPath
        );

        if (success) {

            dbManager.updateObjectTier(
                object.getObjectId(),
                targetTier
            );

            dbManager.updateObjectPath(
                object.getObjectId(),
                targetPath + "/" + object.getFilename()
            );

            Logger::info(
                "Background daemon migrated " +
                object.getFilename() +
                " to " +
                targetTier
            );
            dbManager.insertAuditLog(
                "migrate",
                object.getFilename(),
                object.getTier(),
                targetTier,
                "success",
                "Background daemon moved object"
            );
}