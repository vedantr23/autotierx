#include "../../include/db/DatabaseManager.hpp"

#include <iostream>

namespace autotierx {

DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {

    if (db) {
        sqlite3_close(db);
    }
}

bool DatabaseManager::connect(const std::string& dbPath) {

    int result = sqlite3_open(dbPath.c_str(), &db);

    if (result) {

        std::cout << "Cannot open database!" << std::endl;

        return false;
    }

    std::cout << "SQLite database connected." << std::endl;

    return true;
}

void DatabaseManager::createMetadataTable() {

    const char* sql = R"(

        CREATE TABLE IF NOT EXISTS object_metadata (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            object_id TEXT,
            filename TEXT,
            path TEXT,
            tier TEXT,
            size_bytes INTEGER
        );

    )";

    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {

        std::cout << "SQL Error: " << errorMessage << std::endl;

        sqlite3_free(errorMessage);

    } else {

        std::cout << "Metadata table ready." << std::endl;
    }
}

void DatabaseManager::insertObjectMetadata(
    const std::string& objectId,
    const std::string& filename,
    const std::string& path,
    const std::string& tier,
    long sizeBytes
) {

    std::string sql =

        "INSERT INTO object_metadata "
        "(object_id, filename, path, tier, size_bytes) VALUES ('" +

        objectId + "', '" +
        filename + "', '" +
        path + "', '" +
        tier + "', " +

        std::to_string(sizeBytes) + ");";

    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {

        std::cout << "Insert Error: "
                  << errorMessage
                  << std::endl;

        sqlite3_free(errorMessage);

    } else {

        std::cout << "Metadata inserted into database." << std::endl;
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

    std::cout << "------------------" << std::endl;

    return 0;
}

void DatabaseManager::printAllMetadata() {

    const char* sql =
        "SELECT * FROM object_metadata;";

    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql,
        callback,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {

        std::cout << "Select Error: "
                  << errorMessage
                  << std::endl;

        sqlite3_free(errorMessage);
    }
}

}