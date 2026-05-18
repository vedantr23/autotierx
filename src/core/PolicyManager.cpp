#include "../../include/core/PolicyManager.hpp"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace autotierx {

PolicyManager::PolicyManager()
    : warmDays(7),
      coldDays(15),
      archiveDays(30)
{
}

bool PolicyManager::loadPolicies(
    const std::string& path
) {

    std::ifstream file(path);

    if (!file.is_open()) {

        std::cout
            << "Failed to open policy file: "
            << path
            << std::endl;

        return false;
    }

    json config;

    file >> config;

    extensionPolicies.clear();

    if (config.contains("extensions")) {
        for (auto& item :
             config["extensions"].items()) {

            extensionPolicies[item.key()] =
                item.value();
        }
    }

    if (config.contains("lifecycle")) {

        warmDays =
            config["lifecycle"]["warm_after_days"].get<int>();

        coldDays =
            config["lifecycle"]["cold_after_days"].get<int>();

        archiveDays =
            config["lifecycle"]["archive_after_days"].get<int>();
    }

    return true;
}

std::string PolicyManager::getTierForExtension(
    const std::string& extension
) {

    if (extensionPolicies.count(extension)) {

        return extensionPolicies[extension];
    }

    return "HOT";
}

int PolicyManager::getWarmDays() {

    return warmDays;
}

int PolicyManager::getColdDays() {

    return coldDays;
}

int PolicyManager::getArchiveDays() {

    return archiveDays;
}

}
