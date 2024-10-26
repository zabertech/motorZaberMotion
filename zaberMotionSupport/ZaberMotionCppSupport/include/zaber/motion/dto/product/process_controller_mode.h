// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace product {

/**
 * Servo Tuning Parameter Set to target.
 */
enum class ProcessControllerMode {
    MANUAL = 0,
    PID = 1,
    PID_HEATER = 2,
    ON_OFF = 3,
};

/**
 * Returns string representation of ProcessControllerMode value
 */
std::string ProcessControllerMode_toString(ProcessControllerMode value);


} // namespace product
} // namespace motion
} // namespace zaber
