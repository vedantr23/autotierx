#include "../../include/core/AccessManager.hpp"
#include "../../include/db/DatabaseManager.hpp"

#include <iostream>
#include <chrono>

namespace autotierx {

void AccessManager::accessObject(
    ObjectMetadata& object
) {

    object.incrementAccessCount();

    DatabaseManager dbManager;

    dbManager.connect(
        "/home/vedant/autotierx/metadata/metadata.db"
    );

    dbManager.updateAccessCount(
        object.getObjectId(),
        object.getAccessCount()
    );

    auto now =
        std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

    object.updateLastAccessed(
        std::ctime(&now)
    );

    std::cout << std::endl;

    std::cout
        << "[OBJECT ACCESSED]"
        << std::endl;

    object.printMetadata();
}

}
