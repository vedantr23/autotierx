#ifndef OBJECT_MANAGER_HPP
#define OBJECT_MANAGER_HPP

#include "../db/ObjectMetadata.hpp"
#include <vector>
#include <string>

namespace autotierx {

class ObjectManager {
private:
    std::vector<ObjectMetadata> objects;

public:
    void ingestObject(
        const std::string& sourcePath,
        const std::string& destinationTier
    );

    void printAllObjects() const;
};

}

#endif