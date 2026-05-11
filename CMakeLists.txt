cmake_minimum_required(VERSION 3.10)

project(AutoTierX)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(autotierx
    main.cpp
    src/storage/StorageTier.cpp
    src/storage/StorageManager.cpp
    src/db/ObjectMetadata.cpp
    src/core/ObjectManager.cpp
)