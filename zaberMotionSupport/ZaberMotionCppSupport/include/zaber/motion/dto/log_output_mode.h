// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {

/**
 * Mode of logging output of the library.
 */
enum class LogOutputMode {
    OFF = 0,
    STDOUT = 1,
    STDERR = 2,
    FILE = 3,
};

/**
 * Returns string representation of LogOutputMode value
 */
std::string LogOutputMode_toString(LogOutputMode value);


} // namespace motion
} // namespace zaber
