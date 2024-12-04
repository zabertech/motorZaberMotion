#pragma once

#include <stdexcept>
#include <string>

namespace zaber
{
    namespace motion
    {
        namespace exceptions
        {
            class MotionLibException: public std::runtime_error {
            public:
                MotionLibException(const std::string& message);
                virtual ~MotionLibException() = default;

                /**
                 * Error message of the exception
                **/
                std::string getMessage() const;
            };
        }
    }
}
