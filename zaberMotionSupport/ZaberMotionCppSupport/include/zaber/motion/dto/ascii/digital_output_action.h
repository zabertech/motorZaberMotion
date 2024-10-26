// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Action type for digital output.
 */
enum class DigitalOutputAction {
    OFF = 0,
    ON = 1,
    TOGGLE = 2,
    KEEP = 3,
};

/**
 * Returns string representation of DigitalOutputAction value
 */
std::string DigitalOutputAction_toString(DigitalOutputAction value);


} // namespace ascii
} // namespace motion
} // namespace zaber
