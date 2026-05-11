#pragma once

#include "../db/ObjectMetadata.hpp"

#include <vector>
#include <string>

namespace autotierx {

class AutoTieringEngine {

public:

    void evaluateAndMigrate(
        std::vector<ObjectMetadata>& objects
    );

private:

    std::string determineTargetTier(
        const ObjectMetadata& object
    );
};

}