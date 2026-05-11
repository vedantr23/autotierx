#include "../../include/db/ObjectMetadata.hpp"

namespace autotierx {

ObjectMetadata::ObjectMetadata(
    const std::string& objectId,
    const std::string& filename,
    const std::string& path,
    const std::string& tier,
    long sizeBytes,
    int accessCount,
    const std::string& createdAt,
    const std::string& lastAccessed,
    const std::string& checksum
)
    : objectId(objectId),
      filename(filename),
      path(path),
      tier(tier),
      sizeBytes(sizeBytes),
      accessCount(accessCount),
      createdAt(createdAt),
      lastAccessed(lastAccessed),
      checksum(checksum)
{
}

void ObjectMetadata::printMetadata() const {

    std::cout << "Object ID: "
              << objectId
              << std::endl;

    std::cout << "Filename: "
              << filename
              << std::endl;

    std::cout << "Path: "
              << path
              << std::endl;

    std::cout << "Tier: "
              << tier
              << std::endl;

    std::cout << "Size: "
              << sizeBytes
              << " bytes"
              << std::endl;

    std::cout << "Access Count: "
              << accessCount
              << std::endl;

    std::cout << "Created At: "
              << createdAt
              << std::endl;

    std::cout << "Last Accessed: "
              << lastAccessed
              << std::endl;

    std::cout << "Checksum: "
              << checksum
              << std::endl;
}

/*
=====================================
GETTERS
=====================================
*/

std::string ObjectMetadata::getObjectId() const {
    return objectId;
}

std::string ObjectMetadata::getFilename() const {
    return filename;
}

std::string ObjectMetadata::getPath() const {
    return path;
}

std::string ObjectMetadata::getTier() const {
    return tier;
}

long ObjectMetadata::getSizeBytes() const {
    return sizeBytes;
}

int ObjectMetadata::getAccessCount() const {
    return accessCount;
}

std::string ObjectMetadata::getCreatedAt() const {
    return createdAt;
}

std::string ObjectMetadata::getLastAccessed() const {
    return lastAccessed;
}

std::string ObjectMetadata::getChecksum() const {
    return checksum;
}

/*
=====================================
SETTERS
=====================================
*/

void ObjectMetadata::setTier(
    const std::string& newTier
) {
    tier = newTier;
}

void ObjectMetadata::incrementAccessCount() {
    accessCount++;
}

}