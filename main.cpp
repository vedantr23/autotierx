#include <iostream>
#include "storage/StorageTier.hpp"
#include "storage/StorageManager.hpp"
#include "db/ObjectMetadata.hpp"
#include "core/ObjectManager.hpp"

using namespace autotierx;

int main() {

    StorageManager manager;

    StorageTier hot(
        TierType::HOT,
        "HOT",
        "/home/vedant/hot-storage",
        100,
        true
    );

    StorageTier warm(
        TierType::WARM,
        "WARM",
        "/media/vedant/warm",
        59,
        true
    );

    StorageTier cold(
        TierType::COLD,
        "COLD",
        "/media/vedant/cold",
        14,
        true
    );

    StorageTier archive(
        TierType::ARCHIVE,
        "ARCHIVE",
        "/home/vedant/archive-storage",
        500,
        false
    );

    manager.addTier(hot);
    manager.addTier(warm);
    manager.addTier(cold);
    manager.addTier(archive);

    manager.printAllTiers();

    ObjectMetadata file1(
    "OBJ001",
    "video.mp4",
    "/home/vedant/hot-storage/video.mp4",
    "HOT",
    104857600,
    12,
    "2026-05-11",
    "2026-05-11",
    "abc123checksum"
);

std::cout << std::endl;
std::cout << "===== OBJECT METADATA =====" << std::endl;

file1.printMetadata();

ObjectManager objectManager;

objectManager.ingestObject(
    "/home/vedant/autotierx/input/sample.txt",
    "/home/vedant/hot-storage"
);

objectManager.printAllObjects();

    return 0;
}