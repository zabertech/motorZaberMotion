// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

#include "zaber/motion/ascii/device.h"


namespace zaber {
namespace motion {
namespace ascii {


/**
 * Represents all axes of motion associated with a device.
 */
class AllAxes {
public:
  struct HomeOptions {
    // Determines whether function should return after the movement is finished or just started.
    bool waitUntilIdle {true};
  };

  struct StopOptions {
    // Determines whether function should return after the movement is finished or just started.
    bool waitUntilIdle {true};
  };

  struct WaitUntilIdleOptions {
    // Determines whether to throw error when fault is observed.
    bool throwErrorOnFault {true};
  };

    AllAxes(Device device);

    /**
     * Homes all axes. Axes return to their homing positions.
     * @param waitUntilIdle Determines whether function should return after the movement is finished or just started.
     */
    void home(bool waitUntilIdle = true);

    /**
     * Homes all axes. Axes return to their homing positions.
     * @param options A struct of type HomeOptions. It has the following members:
     * * `waitUntilIdle`: Determines whether function should return after the movement is finished or just started.
     */
    void home(const AllAxes::HomeOptions& options);

    /**
     * Stops ongoing axes movement. Decelerates until zero speed.
     * @param waitUntilIdle Determines whether function should return after the movement is finished or just started.
     */
    void stop(bool waitUntilIdle = true);

    /**
     * Stops ongoing axes movement. Decelerates until zero speed.
     * @param options A struct of type StopOptions. It has the following members:
     * * `waitUntilIdle`: Determines whether function should return after the movement is finished or just started.
     */
    void stop(const AllAxes::StopOptions& options);

    /**
     * Waits until all axes of device stop moving.
     * @param throwErrorOnFault Determines whether to throw error when fault is observed.
     */
    void waitUntilIdle(bool throwErrorOnFault = true);

    /**
     * Waits until all axes of device stop moving.
     * @param options A struct of type WaitUntilIdleOptions. It has the following members:
     * * `throwErrorOnFault`: Determines whether to throw error when fault is observed.
     */
    void waitUntilIdle(const AllAxes::WaitUntilIdleOptions& options);

    /**
     * Parks the device in anticipation of turning the power off.
     * It can later be powered on, unparked, and moved without first having to home it.
     */
    void park();

    /**
     * Unparks the device. The device will now be able to move.
     */
    void unpark();

    /**
     * Returns bool indicating whether any axis is executing a motion command.
     * @return True if any axis is currently executing a motion command.
     */
    bool isBusy();

    /**
     * Returns bool indicating whether all axes have position reference and were homed.
     * @return True if all axes have position reference and were homed.
     */
    bool isHomed();

    /**
     * Returns a string that represents the axes.
     * @return A string that represents the axes.
     */
    std::string toString();

    /**
     * Device that controls this axis.
     */
    Device getDevice() const;

protected:
    Device _device;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
