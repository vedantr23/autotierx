#include "../../include/core/AccessManager.hpp"
#include "../../include/db/DatabaseManager.hpp"
#include "../../include/utils/Logger.hpp"

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

    std::string now = Logger::getCurrentTimestamp();

    object.updateLastAccessed(now);
    dbManager.updateLastAccessed(
        object.getObjectId(),
        now
    );

    std::cout << std::endl;

    std::cout
        << "[OBJECT ACCESSED]"
        << std::endl;

    object.printMetadata();
}

}
