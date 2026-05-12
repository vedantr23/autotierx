#pragma once

#include <string>

namespace autotierx {

class TierUtils {

public:

    static std::string getTierPath(
        const std::string& tier
    );
};

}
