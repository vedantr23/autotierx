#include "../../external/Crow/include/crow/middlewares/cors.h"
#include "../../external/Crow/include/crow.h"
#include <sqlite3.h>
#include <set>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cstdlib>

#include "../../include/api/ApiServer.hpp"
#include "../../include/api/WebsocketBroadcaster.hpp"
#include "../../include/core/ObjectManager.hpp"
#include "../../include/core/AuditLogger.hpp"
#include "../../include/db/DatabaseManager.hpp"
#include "../../include/utils/Logger.hpp"

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

std::set<crow::websocket::connection*> clients;

void broadcastUpdate(const std::string& message) {

    for (auto client : clients) {

        client->send_text(message);
    }
}

void ApiServer::start(
    StorageManager& manager,
    ObjectManager& objectManager,
    AuditLogger& auditLogger
) {

    crow::App<crow::CORSHandler> app;

    auto& cors = app.get_middleware<crow::CORSHandler>();

    cors
        .global()
        .headers("Content-Type", "X-Filename")
        .methods("POST"_method, "GET"_method, "DELETE"_method);

    DatabaseManager startupDbManager;
    if (startupDbManager.connect("/home/vedant/autotierx/metadata/metadata.db")) {
        for (const auto& log : startupDbManager.getAuditLogs()) {
            AuditEvent event;
            event.timestamp = log.timestamp;
            event.eventType = log.event_type;
            event.objectName = log.filename;
            event.details = log.message;
            auditLogger.addEvent(event);
        }
    }

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
([&objectManager, &auditLogger](const crow::request& req) {

    std::string filename;

    try {

        /*
        =========================================
        GET FILENAME FROM HEADER
        =========================================
        */

        filename =
            req.get_header_value("X-Filename");

        if (filename.empty()) {

            Logger::warning("Upload failed: missing X-Filename header");

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

        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        dbManager.insertAuditLog(
            "upload",
            filename,
            "",
            "HOT",
            "success",
            "Uploaded file to HOT tier"
        );

        AuditEvent uploadEvent;
        uploadEvent.timestamp = Logger::getCurrentTimestamp();
        uploadEvent.eventType = "upload";
        uploadEvent.objectName = filename;
        uploadEvent.details = "Uploaded into HOT tier";
        auditLogger.logEvent(uploadEvent);

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

        autotierx::broadcastUpdate("refresh");

        Logger::info("Upload successful: " + filename);

        return crow::response(response);

    } catch (const std::exception& e) {

        Logger::error(std::string("Upload failed for ") + filename + ": " + e.what());

        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        dbManager.insertAuditLog(
            "upload",
            filename,
            "",
            "HOT",
            "failure",
            std::string("Upload failed: ") + e.what()
        );

        AuditEvent failedUploadEvent;
        failedUploadEvent.timestamp = Logger::getCurrentTimestamp();
        failedUploadEvent.eventType = "upload";
        failedUploadEvent.objectName = filename;
        failedUploadEvent.details = std::string("Upload failed: ") + e.what();
        auditLogger.logEvent(failedUploadEvent);

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
([&objectManager, &auditLogger](const std::string& filename) {

    std::string decodedFilename;

    try {

        decodedFilename = urlDecode(filename);

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

            Logger::warning("Download failed: file not found: " + decodedFilename);

            DatabaseManager dbManager;
            dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
            dbManager.insertAuditLog(
                "download",
                decodedFilename,
                "",
                "",
                "failure",
                "File not found"
            );

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

        Logger::info("Download completed: " + decodedFilename + " from " + foundPath);

        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        dbManager.insertAuditLog(
            "download",
            decodedFilename,
            "",
            "",
            "success",
            "Downloaded from " + foundPath
        );

        AuditEvent downloadEvent;
        downloadEvent.timestamp = Logger::getCurrentTimestamp();
        downloadEvent.eventType = "download";
        downloadEvent.objectName = decodedFilename;
        downloadEvent.details = std::string("Downloaded from ") + foundPath;
        auditLogger.logEvent(downloadEvent);

        return response;

    } catch (const std::exception& e) {

        Logger::error(std::string("Download error for ") + decodedFilename + ": " + e.what());
        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        dbManager.insertAuditLog(
            "download",
            decodedFilename,
            "",
            "",
            "failure",
            std::string("Download error: ") + e.what()
        );

        AuditEvent failedDownloadEvent;
        failedDownloadEvent.timestamp = Logger::getCurrentTimestamp();
        failedDownloadEvent.eventType = "download";
        failedDownloadEvent.objectName = decodedFilename;
        failedDownloadEvent.details = std::string("Download error: ") + e.what();
        auditLogger.logEvent(failedDownloadEvent);

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
([&auditLogger](const crow::request& req, const std::string& filename) {
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

            if (deleted || metadataDeleted) {
                Logger::info("Delete completed: " + decodedFilename);
                DatabaseManager dbManager;
                dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
                dbManager.insertAuditLog(
                    "delete",
                    decodedFilename,
                    "",
                    "",
                    "success",
                    "Deleted object and metadata"
                );
            } else {
                Logger::warning("Delete request for missing file: " + decodedFilename);
                DatabaseManager dbManager;
                dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
                dbManager.insertAuditLog(
                    "delete",
                    decodedFilename,
                    "",
                    "",
                    "failure",
                    "File not found for delete"
                );
            }

            autotierx::broadcastUpdate("refresh");

            return resp;

        } catch (const std::exception& e) {

            Logger::error(std::string("Delete error for ") + filename + ": " + e.what());

            crow::response resp(500, e.what());
            resp.set_header("Access-Control-Allow-Origin", "*");
            resp.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            resp.set_header("Access-Control-Allow-Headers", "Content-Type, X-Filename");
            return resp;
        }
    });

    /*
=========================================
WEBSOCKET ROUTE
=========================================
*/

    CROW_WEBSOCKET_ROUTE(app, "/ws")

    .onopen([](crow::websocket::connection& conn) {

        std::cout
            << "WebSocket client connected"
            << std::endl;

        clients.insert(&conn);
    })

    .onclose([](crow::websocket::connection& conn,

                const std::string& reason,

                uint16_t code) {

        std::cout
            << "WebSocket client disconnected"
            << std::endl;

        clients.erase(&conn);
    })

    .onmessage([](crow::websocket::connection& conn,

                  const std::string& data,

                  bool is_binary) {

        conn.send_text("ACK");
    });


    CROW_ROUTE(app, "/migration-history")
    ([]() {

        sqlite3* db;

        crow::json::wvalue response;

        sqlite3_open(

            "/home/vedant/autotierx/metadata/metadata.db",

            &db
        );

        const char* sql =

            "SELECT filename, source_tier, "
            "destination_tier, timestamp "
            "FROM migration_history;";

        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(

            db,

            sql,

            -1,

            &stmt,

            nullptr
        );

        int index = 0;

        while (

            sqlite3_step(stmt)
            == SQLITE_ROW
        ) {

            response[index]["filename"] =

                reinterpret_cast<const char*>(

                    sqlite3_column_text(stmt, 0)
                );

            response[index]["source"] =

                reinterpret_cast<const char*>(

                    sqlite3_column_text(stmt, 1)
                );

            response[index]["destination"] =

                reinterpret_cast<const char*>(

                    sqlite3_column_text(stmt, 2)
                );

            response[index]["timestamp"] =

                reinterpret_cast<const char*>(

                    sqlite3_column_text(stmt, 3)
                );

            index++;
        }

        sqlite3_finalize(stmt);

        sqlite3_close(db);

        return response;
    });

    CROW_ROUTE(app, "/audit-logs")
    ([]() {
        DatabaseManager dbManager;
        dbManager.connect("/home/vedant/autotierx/metadata/metadata.db");
        auto logs = dbManager.getAuditLogs();

        crow::json::wvalue response = crow::json::wvalue::list();
        for (const auto& log : logs) {
            crow::json::wvalue item;
            item["id"] = log.id;
            item["timestamp"] = log.timestamp;
            item["event_type"] = log.event_type;
            item["filename"] = log.filename;
            item["source_tier"] = log.source_tier;
            item["destination_tier"] = log.destination_tier;
            item["status"] = log.status;
            item["message"] = log.message;
            response[response.size()] = std::move(item);
        }

        return response;
    });

    app.port(18080).multithreaded().run();
}

}