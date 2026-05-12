#pragma once

#include "../db/ObjectMetadata.hpp"

namespace autotierx {

class AccessManager {

public:

    void accessObject(
        ObjectMetadata& object
    );
};

}
