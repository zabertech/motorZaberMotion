// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Kind of channel to record in the Oscilloscope.
 */
enum class OscilloscopeDataSource {
    SETTING = 0,
    IO = 1,
};

/**
 * Returns string representation of OscilloscopeDataSource value
 */
std::string OscilloscopeDataSource_toString(OscilloscopeDataSource value);


} // namespace ascii
} // namespace motion
} // namespace zaber
