// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Denotes type of an axis and units it accepts.
 */
enum class AxisType {
    UNKNOWN = 0,
    LINEAR = 1,
    ROTARY = 2,
    PROCESS = 3,
    LAMP = 4,
};

/**
 * Returns string representation of AxisType value
 */
std::string AxisType_toString(AxisType value);


} // namespace ascii
} // namespace motion
} // namespace zaber
