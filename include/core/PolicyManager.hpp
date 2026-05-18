#pragma once

#include <string>
#include <unordered_map>

namespace autotierx {

class PolicyManager {

private:

    std::unordered_map<
        std::string,
        std::string
    > extensionPolicies;

    int warmDays;

    int coldDays;

    int archiveDays;

public:

    PolicyManager();

    bool loadPolicies(
        const std::string& path
    );

    std::string getTierForExtension(
        const std::string& extension
    );

    int getWarmDays();

    int getColdDays();

    int getArchiveDays();
};

}
