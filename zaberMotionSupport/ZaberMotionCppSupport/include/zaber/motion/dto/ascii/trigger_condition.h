// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Comparison operator for trigger condition.
 */
enum class TriggerCondition {
    EQ = 0,
    NE = 1,
    GT = 2,
    GE = 3,
    LT = 4,
    LE = 5,
};

/**
 * Returns string representation of TriggerCondition value
 */
std::string TriggerCondition_toString(TriggerCondition value);


} // namespace ascii
} // namespace motion
} // namespace zaber
