#include "../../include/db/DatabaseManager.hpp"

#include <iostream>

namespace autotierx {

DatabaseManager::DatabaseManager()
    : db(nullptr) {}

DatabaseManager::~DatabaseManager() {

    if (db) {

        sqlite3_close(db);
    }
}

bool DatabaseManager::connect(
    const std::string& dbPath
) {

    int result =
        sqlite3_open(
            dbPath.c_str(),
            &db
        );

    if (result) {

        std::cout
            << "Cannot open database!"
            << std::endl;

        return false;
    }

    std::cout
        << "SQLite database connected."
        << std::endl;

    return true;
}

void DatabaseManager::createMetadataTable() {

    const char* sql = R"(

        CREATE TABLE IF NOT EXISTS object_metadata (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            object_id TEXT UNIQUE,

            filename TEXT,
            path TEXT,
            tier TEXT,
            size_bytes INTEGER,

            access_count INTEGER DEFAULT 0,

            last_accessed TEXT
        );

    )";

    char* errorMessage = nullptr;

    int result =
        sqlite3_exec(
            db,
            sql,
            nullptr,
            nullptr,
            &errorMessage
        );

    if (result != SQLITE_OK) {

        std::cout
            << "SQL Error: "
            << errorMessage
            << std::endl;

        sqlite3_free(errorMessage);

    } else {

        std::cout
            << "Metadata table ready."
            << std::endl;
    }

    const char* migrationTableSQL = R"(

        CREATE TABLE IF NOT EXISTS migration_history (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            filename TEXT,

            source_tier TEXT,

            destination_tier TEXT,

            timestamp TEXT
        );

)";

    sqlite3_exec(

        db,

        migrationTableSQL,

        nullptr,

        nullptr,

        &errorMessage
    );

    const char* auditTableSQL = R"(

        CREATE TABLE IF NOT EXISTS audit_logs (

            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            event_type TEXT,
            filename TEXT,
            source_tier TEXT,
            destination_tier TEXT,
            status TEXT,
            message TEXT
        );

)";

    sqlite3_exec(

        db,

        auditTableSQL,

        nullptr,

        nullptr,

        &errorMessage
    );

    const char* normalizeMetadataSQL = R"(

        UPDATE object_metadata
        SET last_accessed = datetime('now')
        WHERE last_accessed IS NULL
          OR last_accessed NOT GLOB '????-??-?? ??:??:??';

    )";

    sqlite3_exec(
        db,
        normalizeMetadataSQL,
        nullptr,
        nullptr,
        &errorMessage
    );

    const char* normalizeAuditSQL = R"(

        UPDATE audit_logs
        SET timestamp = datetime('now')
        WHERE timestamp IS NULL
          OR timestamp NOT GLOB '????-??-?? ??:??:??';

    )";

    sqlite3_exec(
        db,
        normalizeAuditSQL,
        nullptr,
        nullptr,
        &errorMessage
    );
}

void DatabaseManager::insertObjectMetadata(
    const std::string& objectId,
    const std::string& filename,
    const std::string& path,
    const std::string& tier,
    long sizeBytes,
    int accessCount,
    const std::string& lastAccessed
) {

    std::string sql =

        "INSERT OR REPLACE INTO object_metadata "
        "(object_id, filename, path, tier, size_bytes, access_count, last_accessed) VALUES ('" +

        objectId + "', '" +
        filename + "', '" +
        path + "', '" +
        tier + "', " +

        std::to_string(sizeBytes) + ", " +
        std::to_string(accessCount) + ", '" +
        lastAccessed + "');";

    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {

        std::cout
            << "Insert Error: "
            << errorMessage
            << std::endl;

        sqlite3_free(errorMessage);

    } else {

        std::cout
            << "Metadata inserted into database."
            << std::endl;
    }
}

void DatabaseManager::insertMigrationHistory(

    const std::string& filename,

    const std::string& sourceTier,

    const std::string& destinationTier
) {

    std::string sql =

        "INSERT INTO migration_history "
        "(filename, source_tier, destination_tier, timestamp) VALUES ('" +

        filename + "', '" +

        sourceTier + "', '" +

        destinationTier + "', datetime('now'));";

    char* errorMessage = nullptr;

    sqlite3_exec(

        db,

        sql.c_str(),

        nullptr,

        nullptr,

        &errorMessage
    );
}

void DatabaseManager::insertAuditLog(
    const std::string& eventType,
    const std::string& filename,
    const std::string& sourceTier,
    const std::string& destinationTier,
    const std::string& status,
    const std::string& message
) {
    const char* sql =
        "INSERT INTO audit_logs "
        "(timestamp, event_type, filename, source_tier, destination_tier, status, message) "
        "VALUES (datetime('now'), ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, eventType.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, filename.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, sourceTier.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, destinationTier.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<AuditLogEntry> DatabaseManager::getAuditLogs() {
    std::vector<AuditLogEntry> logs;
    const char* sql =
        "SELECT id, timestamp, event_type, filename, source_tier, destination_tier, status, message "
        "FROM audit_logs ORDER BY id DESC LIMIT 200;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        auto textValue = [&](int column) {
            const unsigned char* rawText = sqlite3_column_text(stmt, column);
            return rawText ? std::string(reinterpret_cast<const char*>(rawText)) : std::string();
        };

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AuditLogEntry entry;
            entry.id = sqlite3_column_int(stmt, 0);
            entry.timestamp = textValue(1);
            entry.event_type = textValue(2);
            entry.filename = textValue(3);
            entry.source_tier = textValue(4);
            entry.destination_tier = textValue(5);
            entry.status = textValue(6);
            entry.message = textValue(7);
            logs.push_back(entry);
        }
        sqlite3_finalize(stmt);
    }
    return logs;
}

static int callback(
    void* NotUsed,
    int argc,
    char** argv,
    char** columnNames
) {

    for (int i = 0; i < argc; i++) {

        std::cout
            << columnNames[i]
            << ": "
            << (argv[i] ? argv[i] : "NULL")
            << std::endl;
    }

    std::cout
        << "------------------"
        << std::endl;

    return 0;
}

void DatabaseManager::printAllMetadata() {

    const char* sql =
        "SELECT * FROM object_metadata;";

    char* errorMessage = nullptr;

    int result =
        sqlite3_exec(
            db,
            sql,
            callback,
            nullptr,
            &errorMessage
        );

    if (result != SQLITE_OK) {

        std::cout
            << "Select Error: "
            << errorMessage
            << std::endl;

        sqlite3_free(errorMessage);
    }
}

/*
==================================================
FETCH ALL OBJECTS FROM DATABASE
==================================================
*/

std::vector<ObjectMetadata>
DatabaseManager::getAllObjects() {

    std::vector<ObjectMetadata> objects;

    sqlite3_stmt* statement;

    std::string sql =

        "SELECT object_id, "
        "filename, "
        "path, "
        "tier, "
        "size_bytes, "
        "access_count, "
        "last_accessed "
        "FROM object_metadata;";

    int result =
        sqlite3_prepare_v2(
            db,
            sql.c_str(),
            -1,
            &statement,
            nullptr
        );

    if (result != SQLITE_OK) {

        std::cout
            << "Failed to fetch metadata!"
            << std::endl;

        return objects;
    }

    while (
        sqlite3_step(statement)
        == SQLITE_ROW
    ) {

        std::string objectId =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 0)
            );

        std::string filename =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 1)
            );

        std::string path =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 2)
            );

        std::string tier =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 3)
            );

        long size =
            sqlite3_column_int64(statement, 4);

        int accessCount =
            sqlite3_column_int(statement, 5);

        std::string lastAccessed =
            reinterpret_cast<const char*>(
                sqlite3_column_text(statement, 6)
            );

        ObjectMetadata metadata(
            objectId,
            filename,
            path,
            tier,
            size,
            accessCount,
            "unknown",
            lastAccessed,
            "checksum"
        );

        objects.push_back(metadata);
    }

    sqlite3_finalize(statement);

    return objects;
}

void DatabaseManager::updateObjectTier(
    const std::string& objectId,
    const std::string& newTier
) {

    std::string sql =

        "UPDATE object_metadata SET tier='" +
        newTier +
        "' WHERE object_id='" +
        objectId + "';";

    char* errorMessage = nullptr;

    sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );
}

void DatabaseManager::updateObjectPath(
    const std::string& objectId,
    const std::string& newPath
) {

    std::string sql =

        "UPDATE object_metadata SET path='" +
        newPath +
        "' WHERE object_id='" +
        objectId + "';";

    char* errorMessage = nullptr;

    sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );
}

void DatabaseManager::updateAccessCount(
    const std::string& objectId,
    int accessCount
) {

    std::string sql =

        "UPDATE object_metadata SET access_count=" +
        std::to_string(accessCount) +
        " WHERE object_id='" +
        objectId + "';";

    char* errorMessage = nullptr;

    sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );
}

void DatabaseManager::updateLastAccessed(
    const std::string& objectId,
    const std::string& timestamp
) {
    std::string sql =
        "UPDATE object_metadata SET last_accessed='" +
        timestamp + "' WHERE object_id='" +
        objectId + "';";

    char* errorMessage = nullptr;

    sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );
}

}