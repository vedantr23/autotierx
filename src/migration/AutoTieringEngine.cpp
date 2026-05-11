#include "../../include/migration/AutoTieringEngine.hpp"

#include <iostream>

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
                << "[AUTO MIGRATION TRIGGERED]"
                << std::endl;

            object.setTier(targetTier);

        } else {

            std::cout
                << "[NO MIGRATION NEEDED]"
                << std::endl;
        }
    }
}

}