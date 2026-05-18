#pragma once

#include "../core/ObjectManager.hpp"
#include "../storage/StorageManager.hpp"
#include "AutoTieringEngine.hpp"

namespace autotierx {

class BackgroundTieringDaemon {

public:

    void start(
        StorageManager& manager,
        ObjectManager& objectManager,
        AutoTieringEngine& autoTieringEngine
    );
};

}
