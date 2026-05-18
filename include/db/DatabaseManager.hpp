#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>
#include <vector>
#include "ObjectMetadata.hpp"

namespace autotierx {

struct AuditLogEntry {
    int id;
    std::string timestamp;
    std::string event_type;
    std::string filename;
    std::string source_tier;
    std::string destination_tier;
    std::string status;
    std::string message;
};

class DatabaseManager {
public:
    std::vector<ObjectMetadata> getAllObjects();
    std::vector<AuditLogEntry> getAuditLogs();

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
    void insertAuditLog(
        const std::string& eventType,
        const std::string& filename,
        const std::string& sourceTier,
        const std::string& destinationTier,
        const std::string& status,
        const std::string& message
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

    void updateLastAccessed(
        const std::string& objectId,
        const std::string& timestamp
    );

    void insertMigrationHistory(

        const std::string& filename,

        const std::string& sourceTier,

        const std::string& destinationTier
    );

    void printAllMetadata();
};

}

#endif