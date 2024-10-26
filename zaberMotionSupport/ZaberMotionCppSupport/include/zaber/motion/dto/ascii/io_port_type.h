// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Kind of I/O pin to use.
 */
enum class IoPortType {
    NONE = 0,
    ANALOG_INPUT = 1,
    ANALOG_OUTPUT = 2,
    DIGITAL_INPUT = 3,
    DIGITAL_OUTPUT = 4,
};

/**
 * Returns string representation of IoPortType value
 */
std::string IoPortType_toString(IoPortType value);


} // namespace ascii
} // namespace motion
} // namespace zaber
