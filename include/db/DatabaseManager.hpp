#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <sqlite3.h>
#include <string>

namespace autotierx {

class DatabaseManager {
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
        long sizeBytes
    );

    void printAllMetadata();
};

}

#endif