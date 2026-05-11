#pragma once

#include <string>

namespace autotierx {

class TieringPolicy {

public:

    static std::string determineTier(
        int accessCount,
        int daysSinceLastAccess
    );
};

}