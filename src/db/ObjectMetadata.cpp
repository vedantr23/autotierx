#include "../../include/db/ObjectMetadata.hpp"
#include <iostream>

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
      checksum(checksum) {}

void ObjectMetadata::printMetadata() const {

    std::cout << "Object ID: " << objectId << std::endl;
    std::cout << "Filename: " << filename << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Tier: " << tier << std::endl;
    std::cout << "Size: " << sizeBytes << " bytes" << std::endl;
    std::cout << "Access Count: " << accessCount << std::endl;
    std::cout << "Created At: " << createdAt << std::endl;
    std::cout << "Last Accessed: " << lastAccessed << std::endl;
    std::cout << "Checksum: " << checksum << std::endl;
}

}