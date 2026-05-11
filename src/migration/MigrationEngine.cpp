#include "../../include/migration/MigrationEngine.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace autotierx {

bool MigrationEngine::migrateObject(
    const std::string& sourcePath,
    const std::string& destinationTier
) {

    try {

        fs::path source(sourcePath);

        if (!fs::exists(source)) {

            std::cout
                << "Source object not found."
                << std::endl;

            return false;
        }

        std::string filename = source.filename();

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

        return true;

    } catch (const std::exception& e) {

        std::cout
            << "Migration Error: "
            << e.what()
            << std::endl;

        return false;
    }
}

}