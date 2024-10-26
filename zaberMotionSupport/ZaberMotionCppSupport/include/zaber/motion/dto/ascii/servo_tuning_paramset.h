// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Servo Tuning Parameter Set to target.
 */
enum class ServoTuningParamset {
    LIVE = 0,
    P_1 = 1,
    P_2 = 2,
    P_3 = 3,
    P_4 = 4,
    P_5 = 5,
    P_6 = 6,
    P_7 = 7,
    P_8 = 8,
    P_9 = 9,
    STAGING = 10,
    DEFAULT = 11,
};

/**
 * Returns string representation of ServoTuningParamset value
 */
std::string ServoTuningParamset_toString(ServoTuningParamset value);


} // namespace ascii
} // namespace motion
} // namespace zaber
