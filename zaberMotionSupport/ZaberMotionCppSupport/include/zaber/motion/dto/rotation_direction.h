// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {

/**
 * Direction of rotation.
 */
enum class RotationDirection {
    CLOCKWISE = 0,
    COUNTERCLOCKWISE = 1,
    CW = 0,
    CCW = 1,
};

/**
 * Returns string representation of RotationDirection value
 */
std::string RotationDirection_toString(RotationDirection value);


} // namespace motion
} // namespace zaber
