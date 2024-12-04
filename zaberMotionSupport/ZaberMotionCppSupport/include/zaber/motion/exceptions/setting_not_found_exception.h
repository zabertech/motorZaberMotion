// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/exceptions/motion_lib_exception.h"
#include <string>

namespace zaber
{
    namespace motion
    {
        namespace exceptions
        {
            /**
             * Thrown when a get or a set command cannot be found for a setting.
             */
            class SettingNotFoundException: public MotionLibException {
            public:
                SettingNotFoundException(const std::string& message);
                virtual ~SettingNotFoundException() = default;
            };
        }
    }
}
