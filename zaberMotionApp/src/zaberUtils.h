#ifndef ZABER_UTILS_H
#define ZABER_UTILS_H

#include <functional>
#include <iostream>

#include <asynDriver.h>

namespace zaber {
namespace motion {

inline asynStatus performAction(
        const std::function<asynStatus()> &action, const std::function<void()> &onError = nullptr) {
    asynStatus status = asynSuccess;
    try {
        status = action();
    } catch (const std::exception &e) {
        std::cerr << "Zaber Motion Error: " << e.what() << std::endl;
        status = asynError;
    }

    if (status != asynSuccess && onError) {
        try {
            onError();
        } catch (const std::exception &e) {
            // if onError callback fails, do it silently (the user doesn't need to know)
        }
    }
    return status;
}

} // namespace motion
} // namespace zaber
#endif