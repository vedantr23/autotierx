#ifndef STORAGE_MANAGER_HPP
#define STORAGE_MANAGER_HPP

#include "StorageTier.hpp"
#include <vector>

namespace autotierx {

class StorageManager {
private:
    std::vector<StorageTier> tiers;

public:
    void addTier(const StorageTier& tier);
    void printAllTiers() const;
    const std::vector<StorageTier>& getTiers() const;
    void refreshTierHealth();
};

}

#endif