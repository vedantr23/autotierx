cmake_minimum_required(VERSION 3.10)

project(AutoTierX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(external/Crow/include)

find_package(nlohmann_json REQUIRED)

add_executable(autotierx
    main.cpp
    src/storage/StorageTier.cpp
    src/storage/StorageManager.cpp
    src/db/ObjectMetadata.cpp
    src/core/ObjectManager.cpp
    src/core/AccessManager.cpp
    src/core/AuditLogger.cpp
    src/db/DatabaseManager.cpp
    src/migration/MigrationEngine.cpp
    src/classifier/TieringPolicy.cpp
    src/migration/AutoTieringEngine.cpp
    src/classification/ClassificationEngine.cpp
    src/core/PolicyManager.cpp
    src/core/BackgroundTieringDaemon.cpp
    src/utils/Logger.cpp
    src/utils/TierUtils.cpp
    src/monitoring/MonitoringEngine.cpp
    src/api/ApiServer.cpp
)

target_link_libraries(autotierx sqlite3 nlohmann_json::nlohmann_json)