#ifndef MIGRATION_ENGINE_HPP
#define MIGRATION_ENGINE_HPP

#include <string>

namespace autotierx {

class MigrationEngine {
public:

    bool migrateObject(
        const std::string& sourcePath,
        const std::string& destinationTier
    );
};

}

#endif