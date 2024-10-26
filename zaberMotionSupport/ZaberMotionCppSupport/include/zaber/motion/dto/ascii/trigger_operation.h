// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Operation for trigger action.
 */
enum class TriggerOperation {
    SET_TO = 0,
    INCREMENT_BY = 1,
    DECREMENT_BY = 2,
};

/**
 * Returns string representation of TriggerOperation value
 */
std::string TriggerOperation_toString(TriggerOperation value);


} // namespace ascii
} // namespace motion
} // namespace zaber
