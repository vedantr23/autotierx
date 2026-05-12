cmake_minimum_required(VERSION 3.10)

project(AutoTierX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
include_directories(external/Crow/include)

add_executable(autotierx
    main.cpp
    src/storage/StorageTier.cpp
    src/storage/StorageManager.cpp
    src/db/ObjectMetadata.cpp
    src/core/ObjectManager.cpp
    src/core/AccessManager.cpp
    src/db/DatabaseManager.cpp
    src/migration/MigrationEngine.cpp
    src/classifier/TieringPolicy.cpp
    src/migration/AutoTieringEngine.cpp
    src/utils/Logger.cpp
    src/utils/TierUtils.cpp
    src/monitoring/MonitoringEngine.cpp
    src/api/ApiServer.cpp
)

target_link_libraries(autotierx sqlite3)