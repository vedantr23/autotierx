#pragma once

#include "../db/ObjectMetadata.hpp"

#include <vector>
#include <string>

namespace autotierx {

class ObjectManager {

public:

    void ingestObject(
        const std::string& sourcePath,
        const std::string& destinationTier
    );

    void printAllObjects() const;

    std::vector<ObjectMetadata>& getObjects();

private:

    std::vector<ObjectMetadata> objects;
};

}