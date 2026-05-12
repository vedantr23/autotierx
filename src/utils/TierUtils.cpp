#include "../../include/utils/TierUtils.hpp"

namespace autotierx {

std::string TierUtils::getTierPath(
    const std::string& tier
) {

    if (tier == "HOT") {

        return "/home/vedant/hot-storage";
    }

    if (tier == "WARM") {

        return "/media/vedant/warm";
    }

    if (tier == "COLD") {

        return "/media/vedant/cold";
    }

    return "/home/vedant/archive-storage";
}

}
