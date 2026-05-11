#ifndef STORAGE_TIER_HPP
#define STORAGE_TIER_HPP

#include <string>

namespace autotierx {

enum class TierType {
    HOT,
    WARM,
    COLD,
    ARCHIVE
};

class StorageTier {
private:
    TierType type;
    std::string name;
    std::string path;
    double capacityGB;
    bool online;

public:
    StorageTier(
        TierType type,
        const std::string& name,
        const std::string& path,
        double capacityGB,
        bool online
    );

    std::string getName() const;
    std::string getPath() const;
    double getCapacity() const;
    bool isOnline() const;
    TierType getType() const;

    void printInfo() const;
};

}

#endif