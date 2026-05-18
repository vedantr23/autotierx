#include "../../include/classification/ClassificationEngine.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace autotierx {

std::string ClassificationEngine::determineOptimalTier(

    const ObjectMetadata& object
) {

    std::ifstream configFile(

        "/home/vedant/autotierx/config/policies.json"
    );

    nlohmann::json config;

    if (configFile.good()) {

        try {
            configFile >> config;
        } catch (...) {
            // fall through to default
        }
    }

    fs::path filePath(object.getPath());

    std::string extension =
        filePath.extension();

    /*
    =========================================
    POLICY MATCH
    =========================================
    */

    if (!config.is_null() &&
        config.contains("extensions") &&
        config["extensions"].contains(extension)) {

        try {
            return config["extensions"][extension].get<std::string>();
        } catch (...) {
            // ignore and fallthrough
        }
    }

    /*
    =========================================
    DEFAULT
    =========================================
    */

    return "HOT";
}

}
