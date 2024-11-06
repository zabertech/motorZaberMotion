#ifndef ZABER_UTILS_H
#define ZABER_UTILS_H

#include <asynDriver.h>
#include <functional>
#include <iostream>

#include <zaber/motion/exceptions/motion_lib_exception.h>

namespace zml = zaber::motion;

namespace zaber {
namespace epics {

inline asynStatus handleException(
    const std::function<asynStatus()> &action, const std::function<void()> &onError = nullptr) {
    asynStatus status = asynSuccess;
    try {
        status = action();
    } catch(const zml::exceptions::MotionLibException &e) {
        std::cerr << "Zaber Motion Library Error: " << e.what() << std::endl;
        status = asynError;
    } catch(const std::exception &e) {
        std::cerr << "Zaber Motion Motor Error: " << e.what() << std::endl;
        status = asynError;
    }

	if (status == asynSuccess || !onError) {
		return status;
	}

	try {
		onError();
	} catch(const std::exception &e) {
		// if onError callback throws exception, do it silently (the user doesn't need to know)
	}
    return status;
}

} // namespace motion
} // namespace zaber

#endif
