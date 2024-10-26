// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Trigger action identifier.
 */
enum class TriggerAction {
    ALL = 0,
    A = 1,
    B = 2,
};

/**
 * Returns string representation of TriggerAction value
 */
std::string TriggerAction_toString(TriggerAction value);


} // namespace ascii
} // namespace motion
} // namespace zaber
