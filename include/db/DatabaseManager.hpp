#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include "ObjectMetadata.hpp"

namespace autotierx {

class DatabaseManager {
    std::vector<ObjectMetadata> fetchAllObjects();

private:
    sqlite3* db;

public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const std::string& dbPath);

    void createMetadataTable();

    void insertObjectMetadata(
        const std::string& objectId,
        const std::string& filename,
        const std::string& path,
        const std::string& tier,
        long sizeBytes,
        int accessCount,
        const std::string& lastAccessed
    );

    void updateObjectTier(
        const std::string& objectId,
        const std::string& newTier
    );

    void updateObjectPath(
        const std::string& objectId,
        const std::string& newPath
    );

    void updateAccessCount(
        const std::string& objectId,
        int accessCount
    );

    void printAllMetadata();
};

}

#endif