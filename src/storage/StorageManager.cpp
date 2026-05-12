#include "../../include/storage/StorageManager.hpp"
#include <iostream>

namespace autotierx {

void StorageManager::addTier(const StorageTier& tier) {
    tiers.push_back(tier);
}

void StorageManager::printAllTiers() const {
    for (const auto& tier : tiers) {
        tier.printInfo();
        std::cout << "------------------" << std::endl;
    }
}

const std::vector<StorageTier>&
StorageManager::getTiers() const {

    return tiers;
}

}