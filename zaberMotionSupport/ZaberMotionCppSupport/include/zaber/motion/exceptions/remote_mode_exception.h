// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/exceptions/command_failed_exception.h"
#include "zaber/motion/dto/exceptions/command_failed_exception_data.h"
#include <string>

namespace zaber
{
    namespace motion
    {
        namespace exceptions
        {
            /**
             * Thrown when a command is rejected because the device is in EtherCAT Control (remote) mode.
             */
            class RemoteModeException: public CommandFailedException {
            public:
                RemoteModeException(const std::string& message, const std::string &customData);
                RemoteModeException(const std::string& message, const CommandFailedExceptionData &&customData);
                virtual ~RemoteModeException() = default;
            };
        }
    }
}
