// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Mode of a PVT sequence.
 */
enum class PvtMode {
    DISABLED = 0,
    STORE = 1,
    LIVE = 2,
};

/**
 * Returns string representation of PvtMode value
 */
std::string PvtMode_toString(PvtMode value);


} // namespace ascii
} // namespace motion
} // namespace zaber
