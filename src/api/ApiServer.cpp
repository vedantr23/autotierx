#include "../../external/Crow/include/crow/middlewares/cors.h"
#include "../../external/Crow/include/crow.h"
#include <sqlite3.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "../../include/api/ApiServer.hpp"
#include "../../include/core/ObjectManager.hpp"

static std::string urlDecode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.size());

    for (size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.size()) {
            std::string hex = encoded.substr(i + 1, 2);
            char ch = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            decoded.push_back(ch);
            i += 2;
        } else if (encoded[i] == '+') {
            decoded.push_back(' ');
        } else {
            decoded.push_back(encoded[i]);
        }
    }

    return decoded;
}

static std::vector<std::string> getStoragePaths(const std::string& filename) {
    return {
        "/home/vedant/hot-storage/" + filename,
        "/media/vedant/warm/" + filename,
        "/media/vedant/cold/" + filename,
        "/home/vedant/archive-storage/" + filename,
        "/home/vedant/autotierx/uploads/" + filename
    };
}

static std::string findExistingPath(const std::string& filename) {
    for (const auto& path : getStoragePaths(filename)) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return {};
}

namespace autotierx {

void ApiServer::start(
    StorageManager& manager,
    ObjectManager& objectManager
) {

    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors
        .global()
        .headers("Content-Type", "X-Filename")
        .methods("POST"_method, "GET"_method, "DELETE"_method);

    /*
    =========================================
    ROOT ROUTE
    =========================================
    */

    CROW_ROUTE(app, "/")
    ([](){

        return "AutoTierX API Server Running";
    });

    /*
    =========================================
    STORAGE TIERS ROUTE
    =========================================
    */

    CROW_ROUTE(app, "/tiers")
    ([&manager]() {

        crow::json::wvalue response = crow::json::wvalue::list();

        auto tiers = manager.getTiers();

        for (const auto& tier : tiers) {

            crow::json::wvalue item;

            item["name"] = tier.getName();
            item["path"] = tier.getPath();
            item["capacity"] = tier.getCapacity();
            item["status"] = tier.isAvailable()
                ? "ONLINE"
                : "OFFLINE";

            response[response.size()] = std::move(item);
        }

        return response;
    });

    /*
    =========================================
    STORED OBJECTS ROUTE
    =========================================
    */

    CROW_ROUTE(app, "/objects")
    ([]() {

        sqlite3* db;

        crow::json::wvalue response = crow::json::wvalue::list();

        int result = sqlite3_open(
            "/home/vedant/autotierx/metadata/metadata.db",
            &db
        );

        if (result != SQLITE_OK) {

            return response;
        }

        const char* sql =
            "SELECT object_id, filename, tier, "
            "size_bytes, access_count, "
            "last_accessed FROM object_metadata;";

        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        );

        while (
            sqlite3_step(stmt)
            == SQLITE_ROW
        ) {

            std::string filename = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 1)
            );

            if (filename.empty()) {
                continue;
            }

            std::string actualPath = findExistingPath(filename);
            if (actualPath.empty()) {
                const char* deleteSql =
                    "DELETE FROM object_metadata WHERE filename = ?;";
                sqlite3_stmt* deleteStmt = nullptr;
                if (sqlite3_prepare_v2(db, deleteSql, -1, &deleteStmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(
                        deleteStmt,
                        1,
                        filename.c_str(),
                        -1,
                        SQLITE_TRANSIENT
                    );
                    sqlite3_step(deleteStmt);
                    sqlite3_finalize(deleteStmt);
                }
                continue;
            }

            crow::json::wvalue item;

            item["object_id"] = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 0)
            );

            item["filename"] = filename;

            item["tier"] = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 2)
            );

            item["size_bytes"] = sqlite3_column_int(stmt, 3);
            item["access_count"] = sqlite3_column_int(stmt, 4);
            item["last_accessed"] = reinterpret_cast<const char*>(
                sqlite3_column_text(stmt, 5)
            );

            response[response.size()] = std::move(item);
        }

        sqlite3_finalize(stmt);

        sqlite3_close(db);

        return response;
    });

    /*
    =========================================
    HEALTH ROUTE
    =========================================
    */

    CROW_ROUTE(app, "/health")
    ([]() {

        crow::json::wvalue response;

        response["status"] = "healthy";

        response["server"] = "AutoTierX";

        response["version"] = "1.0";

        return response;
    });

    /*
    =========================================
    METRICS ROUTE
    =========================================
    */

    CROW_ROUTE(app, "/metrics")
    ([&manager]() {

        crow::json::wvalue response;

        auto tiers = manager.getTiers();

        int totalCapacity = 0;

        int onlineTiers = 0;

        for (const auto& tier : tiers) {

            totalCapacity +=
                tier.getCapacity();

            if (tier.isAvailable()) {

                onlineTiers++;
            }
        }

        response["total_tiers"] =
            tiers.size();

        response["online_tiers"] =
            onlineTiers;

        response["total_capacity_gb"] =
            totalCapacity;

        return response;
    });

        /*
=========================================
UPLOAD ROUTE
=========================================
*/

CROW_ROUTE(app, "/upload")
.methods("POST"_method)
([&objectManager](const crow::request& req) {

    try {

        /*
        =========================================
        GET FILENAME FROM HEADER
        =========================================
        */

        std::string filename =
            req.get_header_value("X-Filename");

        if (filename.empty()) {

            crow::json::wvalue errorResponse;

            errorResponse["status"] =
                "error";

            errorResponse["message"] =
                "Filename header missing";

            return crow::response(
                400,
                errorResponse
            );
        }

        /*
        =========================================
        TEMP FILE PATH
        =========================================
        */

        std::string tempPath =
            "/home/vedant/autotierx/uploads/"
            + filename;

        /*
        =========================================
        SAVE REAL FILE CONTENT
        =========================================
        */

        std::ofstream outputFile(
            tempPath,
            std::ios::binary
        );

        outputFile << req.body;

        outputFile.close();

        /*
        =========================================
        INGEST INTO HOT TIER
        =========================================
        */

        objectManager.ingestObject(
            tempPath,
            "/home/vedant/hot-storage"
        );

        /*
        =========================================
        SUCCESS RESPONSE
        =========================================
        */

        crow::json::wvalue response;

        response["status"] =
            "success";

        response["message"] =
            "File uploaded successfully";

        response["filename"] =
            filename;

        return crow::response(response);

    } catch (const std::exception& e) {

        crow::json::wvalue errorResponse;

        errorResponse["status"] =
            "error";

        errorResponse["message"] =
            e.what();

        return crow::response(
            500,
            errorResponse
        );
    }
});

/*
=========================================
DOWNLOAD ROUTE
=========================================
*/

CROW_ROUTE(app, "/download/<string>")
([&objectManager](const std::string& filename) {

    try {

        std::string decodedFilename = urlDecode(filename);

        /*
        =========================================
        SEARCH POSSIBLE TIER PATHS
        =========================================
        */

        std::vector<std::string> paths = {

            "/home/vedant/hot-storage/" + decodedFilename,

            "/media/vedant/warm/" + decodedFilename,

            "/media/vedant/cold/" + decodedFilename,

            "/home/vedant/archive-storage/" + decodedFilename,

            "/home/vedant/autotierx/uploads/" + decodedFilename
        };

        std::string foundPath;

        /*
        =========================================
        FIND FILE
        =========================================
        */

        for (const auto& path : paths) {

            if (std::filesystem::exists(path)) {

                foundPath = path;

                break;
            }
        }

        /*
        =========================================
        FILE NOT FOUND
        =========================================
        */

        if (foundPath.empty()) {

            return crow::response(
                404,
                "File not found"
            );
        }

        /*
        =========================================
        READ REAL FILE
        =========================================
        */

        std::ifstream file(
            foundPath,
            std::ios::binary
        );

        std::ostringstream buffer;

        buffer << file.rdbuf();

        /*
        =========================================
        RETURN FILE
        =========================================
        */

        crow::response response;

        response.code = 200;

        response.set_header(
            "Content-Type",
            "application/octet-stream"
        );

        response.set_header(
            "Content-Disposition",
            "attachment; filename=\"" +
            decodedFilename + "\""
        );

        response.body = buffer.str();

        return response;

    } catch (const std::exception& e) {

        return crow::response(
            500,
            e.what()
        );
    }
});

/*
=========================================
DELETE OBJECT ROUTE
=========================================
*/

CROW_ROUTE(app, "/delete/<string>")
    .methods("DELETE"_method, "OPTIONS"_method)
    ([](const crow::request& req, const std::string& filename) {

        if (req.method == crow::HTTPMethod::Options) {
            crow::response resp;
            resp.code = 204;
            resp.set_header("Access-Control-Allow-Origin", "*");
            resp.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            resp.set_header("Access-Control-Allow-Headers", "Content-Type, X-Filename");
            return resp;
        }

        try {

            std::string decodedFilename = urlDecode(filename);

            /*
            =========================================
            POSSIBLE STORAGE PATHS
            =========================================
            */

            std::vector<std::string> paths = {

                "/home/vedant/hot-storage/" + decodedFilename,

                "/media/vedant/warm/" + decodedFilename,

                "/media/vedant/cold/" + decodedFilename,

                "/home/vedant/archive-storage/" + decodedFilename,

                "/home/vedant/autotierx/uploads/" + decodedFilename
            };

            /*
            =========================================
            DELETE REAL FILE
            =========================================
            */

            bool deleted = false;

            for (const auto& path : paths) {

                if (std::filesystem::exists(path)) {

                    std::filesystem::remove(path);

                    deleted = true;
                }
            }

            /*
            =========================================
            REMOVE SQLITE METADATA
            =========================================
            */

            sqlite3* db;

            sqlite3_open(
                "/home/vedant/autotierx/metadata/metadata.db",
                &db
            );

            const char* sql =
                "DELETE FROM object_metadata "
                "WHERE filename = ?;";

            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr
            );

            sqlite3_bind_text(
                stmt,
                1,
                decodedFilename.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_step(stmt);
            bool metadataDeleted = sqlite3_changes(db) > 0;
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            /*
            =========================================
            RESPONSE
            =========================================
            */

            crow::json::wvalue response;

            response["status"] =
                (deleted || metadataDeleted)
                ? "deleted"
                : "not_found";

            response["filename"] =
                filename;

            crow::response resp(response);
            
            resp.set_header("Access-Control-Allow-Origin", "*");
            resp.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            resp.set_header("Access-Control-Allow-Headers", "Content-Type, X-Filename");

            return resp;

        } catch (const std::exception& e) {

            crow::response resp(500, e.what());
            resp.set_header("Access-Control-Allow-Origin", "*");
            resp.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            resp.set_header("Access-Control-Allow-Headers", "Content-Type, X-Filename");
            return resp;
        }
    });

    app.port(18080).multithreaded().run();
}

}