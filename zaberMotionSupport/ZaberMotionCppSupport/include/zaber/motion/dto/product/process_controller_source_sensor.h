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
enum class ProcessControllerSourceSensor {
    THERMISTOR = 10,
    ANALOG_INPUT = 20,
};

/**
 * Returns string representation of ProcessControllerSourceSensor value
 */
std::string ProcessControllerSourceSensor_toString(ProcessControllerSourceSensor value);


} // namespace product
} // namespace motion
} // namespace zaber
