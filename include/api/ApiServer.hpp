#pragma once

#include "../storage/StorageManager.hpp"
#include "../core/ObjectManager.hpp"

namespace autotierx {

class ApiServer {

public:

    void start(
        StorageManager& manager,
        ObjectManager& objectManager
    );
};

}