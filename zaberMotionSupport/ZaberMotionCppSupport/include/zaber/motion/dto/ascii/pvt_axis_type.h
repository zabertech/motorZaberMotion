// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Denotes type of the PVT sequence axis.
 */
enum class PvtAxisType {
    PHYSICAL = 0,
    LOCKSTEP = 1,
};

/**
 * Returns string representation of PvtAxisType value
 */
std::string PvtAxisType_toString(PvtAxisType value);


} // namespace ascii
} // namespace motion
} // namespace zaber
