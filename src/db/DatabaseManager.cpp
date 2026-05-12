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
DatabaseManager::fetchAllObjects() {

    std::vector<ObjectMetadata> objects;

    sqlite3_stmt* statement;

    std::string sql =

        "SELECT object_id, "
        "filename, "
        "path, "
        "tier, "
        "size_bytes "
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

        ObjectMetadata metadata(
            objectId,
            filename,
            path,
            tier,
            size,
            0,
            "unknown",
            "unknown",
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

}