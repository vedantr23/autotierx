#pragma once

#include "../db/ObjectMetadata.hpp"

namespace autotierx {

class ClassificationEngine {

public:

    std::string determineOptimalTier(

        const ObjectMetadata& object
    );
};

}
