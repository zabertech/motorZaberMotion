// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <unordered_set>

#include "zaber/motion/ascii/device.h"


namespace zaber {
namespace motion {
namespace ascii {


/**
 * Class used to check and reset warnings and faults on device or axis.
 */
class Warnings {
public:
    Warnings(Device device, int axisNumber);

    /**
     * Returns current warnings and faults on axis or device.
     * @return Retrieved warnings and faults. Refer to WarningFlags to check a particular flag.
     */
    std::unordered_set<std::string> getFlags();

    /**
     * Clears (acknowledges) current warnings and faults on axis or device and returns them.
     * @return Warnings and faults before clearing. Refer to WarningFlags to check a particular flag.
     */
    std::unordered_set<std::string> clearFlags();

    /**
     * Waits for the specified flags to clear.
     * Use for warnings flags that clear on their own.
     * Does not clear clearable warnings flags.
     * Throws TimeoutException if the flags don't clear in the specified time.
     * @param timeout For how long to wait in milliseconds for the flags to clear.
     * @param warningFlags The specific warning flags for which to wait to clear.
     */
    void waitToClear(double timeout, std::initializer_list<std::string> warningFlags);

    template<typename... T>
    void waitToClear(double timeout, T&&... warningFlags) {
        return waitToClear(timeout, {std::forward<T>(warningFlags)...});
    }

protected:
    Device getDevice() const;

    int getAxisNumber() const;

    Device _device;
    int _axisNumber;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
