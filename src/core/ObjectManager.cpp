#include "../../include/core/ObjectManager.hpp"
#include "../../include/db/DatabaseManager.hpp"

#include <filesystem>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

namespace autotierx {

void ObjectManager::ingestObject(
    const std::string& sourcePath,
    const std::string& destinationTier
) {

    try {

        fs::path source(sourcePath);

        if (!fs::exists(source)) {

            std::cout
                << "File does not exist!"
                << std::endl;

            return;
        }

        std::string filename =
            source.filename();

        std::string destinationPath =
            destinationTier + "/" + filename;

        /*
        =====================================
        COPY FILE TO STORAGE TIER
        =====================================
        */

        fs::copy_file(
            source,
            destinationPath,
            fs::copy_options::overwrite_existing
        );

        /*
        =====================================
        GET FILE SIZE
        =====================================
        */

        long size =
            fs::file_size(destinationPath);

        /*
        =====================================
        GET CURRENT TIME
        =====================================
        */

        auto now =
            std::chrono::system_clock::to_time_t(
                std::chrono::system_clock::now()
            );

        /*
        =====================================
        CREATE METADATA OBJECT
        =====================================
        */

        ObjectMetadata metadata(
            "OBJ_" + filename,
            filename,
            destinationPath,
            "HOT",
            size,
            0,
            std::ctime(&now),
            std::ctime(&now),
            "dummy-checksum"
        );

        /*
        =====================================
        STORE IN MEMORY
        =====================================
        */

        objects.push_back(metadata);

        /*
        =====================================
        DATABASE OPERATIONS
        =====================================
        */

        DatabaseManager databaseManager;

        databaseManager.connect(
            "/home/vedant/autotierx/metadata/metadata.db"
        );

        databaseManager.createMetadataTable();

        databaseManager.insertObjectMetadata(
            "OBJ_" + filename,
            filename,
            destinationPath,
            "HOT",
            size,
            0,
            std::ctime(&now)
        );

        /*
        =====================================
        SUCCESS LOGS
        =====================================
        */

        std::cout << std::endl;

        std::cout
            << "[UPLOAD SUCCESS]"
            << std::endl;

        metadata.printMetadata();

    } catch (const std::exception& e) {

        std::cout
            << "Error: "
            << e.what()
            << std::endl;
    }
}

void ObjectManager::printAllObjects() const {

    std::cout << std::endl;

    std::cout
        << "===== STORED OBJECTS ====="
        << std::endl;

    for (const auto& object : objects) {

        object.printMetadata();

        std::cout
            << "--------------------"
            << std::endl;
    }
}

/*
=====================================
GETTER FUNCTION
=====================================
*/

std::vector<ObjectMetadata>&
ObjectManager::getObjects() {

    return objects;
}

}