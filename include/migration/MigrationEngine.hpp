#ifndef MIGRATION_ENGINE_HPP
#define MIGRATION_ENGINE_HPP

#include <string>
#include "../db/ObjectMetadata.hpp"

namespace autotierx {

class MigrationEngine {
public:

    void migrateObject(
        ObjectMetadata& object,
        const std::string& destinationTier
    );
};

}

#endif