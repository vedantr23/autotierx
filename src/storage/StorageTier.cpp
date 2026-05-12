#include "../../include/storage/StorageTier.hpp"
#include <iostream>

namespace autotierx {

StorageTier::StorageTier(
    TierType type,
    const std::string& name,
    const std::string& path,
    double capacityGB,
    bool online
)
    : type(type),
      name(name),
      path(path),
      capacityGB(capacityGB),
      online(online) {}

std::string StorageTier::getName() const {
    return name;
}

std::string StorageTier::getPath() const {
    return path;
}

int StorageTier::getCapacity() const {
    return static_cast<int>(capacityGB);
}

bool StorageTier::isAvailable() const {
    return online;
}

bool StorageTier::isOnline() const {
    return online;
}

TierType StorageTier::getType() const {
    return type;
}

void StorageTier::printInfo() const {
    std::cout << "Tier: " << name << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Capacity: " << capacityGB << " GB" << std::endl;
    std::cout << "Status: " << (online ? "ONLINE" : "OFFLINE") << std::endl;
}

}