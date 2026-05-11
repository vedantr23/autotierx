#include "../../include/classifier/TieringPolicy.hpp"

namespace autotierx {

std::string TieringPolicy::determineTier(
    int accessCount,
    int daysSinceLastAccess
) {

    /*
    ==========================================
    HOT TIER
    ==========================================
    */

    if (accessCount >= 10 &&
        daysSinceLastAccess <= 2) {

        return "HOT";
    }

    /*
    ==========================================
    WARM TIER
    ==========================================
    */

    if (accessCount >= 3 &&
        daysSinceLastAccess <= 7) {

        return "WARM";
    }

    /*
    ==========================================
    COLD TIER
    ==========================================
    */

    if (daysSinceLastAccess <= 30) {

        return "COLD";
    }

    /*
    ==========================================
    ARCHIVE
    ==========================================
    */

    return "ARCHIVE";
}

}