// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace binary {

/**
 * Denotes type of an device and units it accepts.
 */
enum class DeviceType {
    UNKNOWN = 0,
    LINEAR = 1,
    ROTARY = 2,
};

/**
 * Returns string representation of DeviceType value
 */
std::string DeviceType_toString(DeviceType value);


} // namespace binary
} // namespace motion
} // namespace zaber
