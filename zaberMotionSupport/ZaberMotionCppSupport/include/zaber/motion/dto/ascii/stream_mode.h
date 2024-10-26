// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Mode of a stream.
 */
enum class StreamMode {
    DISABLED = 0,
    STORE = 1,
    STORE_ARBITRARY_AXES = 2,
    LIVE = 3,
};

/**
 * Returns string representation of StreamMode value
 */
std::string StreamMode_toString(StreamMode value);


} // namespace ascii
} // namespace motion
} // namespace zaber
