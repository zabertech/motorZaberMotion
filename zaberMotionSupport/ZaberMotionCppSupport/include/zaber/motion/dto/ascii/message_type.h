// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Denotes type of the response message.
 * For more information refer to:
 * [ASCII Protocol Manual](https://www.zaber.com/protocol-manual#topic_message_format).
 */
enum class MessageType {
    REPLY = 0,
    INFO = 1,
    ALERT = 2,
};

/**
 * Returns string representation of MessageType value
 */
std::string MessageType_toString(MessageType value);


} // namespace ascii
} // namespace motion
} // namespace zaber
