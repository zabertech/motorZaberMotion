// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <vector>

#include "zaber/motion/units.h"
#include "zaber/motion/ascii/axis.h"


namespace zaber {
namespace motion {
namespace ascii {

/**
 * Groups multiple axes across devices into a single group to allow for simultaneous movement.
 * Note that the movement is not coordinated and trajectory is inconsistent and not repeatable between calls.
 * Make sure that any possible trajectory is clear of potential obstacles.
 * The movement methods return after all the axes finish the movement successfully
 * or throw an error as soon as possible.
 */
class AxisGroup {
public:
    /**
     * Initializes the group with the axes to be controlled.
     */
    AxisGroup(std::vector<Axis> axes);

    AxisGroup();

    /**
     * Homes the axes.
     */
    void home();

    /**
     * Stops the axes.
     */
    void stop();

    /**
     * Moves the axes to absolute position.
     * @param position Position.
     */
    void moveAbsolute(std::initializer_list<Measurement> position);

    template<typename... T>
    void moveAbsolute(T&&... position) {
        return moveAbsolute({std::forward<T>(position)...});
    }

    /**
     * Move axes to position relative to the current position.
     * @param position Position.
     */
    void moveRelative(std::initializer_list<Measurement> position);

    template<typename... T>
    void moveRelative(T&&... position) {
        return moveRelative({std::forward<T>(position)...});
    }

    /**
     * Moves axes to the minimum position as specified by limit.min.
     */
    void moveMin();

    /**
     * Moves axes to the maximum position as specified by limit.max.
     */
    void moveMax();

    /**
     * Waits until all the axes stop moving.
     */
    void waitUntilIdle();

    /**
     * Returns bool indicating whether any of the axes is executing a motion command.
     * @return True if any of the axes is currently executing a motion command. False otherwise.
     */
    bool isBusy();

    /**
     * Returns bool indicating whether all the axes are homed.
     * @return True if all the axes are homed. False otherwise.
     */
    bool isHomed();

    /**
     * Returns current axes position.
     * The positions are requested sequentially.
     * The result position may not be accurate if the axes are moving.
     * @param unit Units of position. You can specify units once or for each axis separately.
     * @return Axes position.
     */
    std::vector<double> getPosition(std::initializer_list<Units> unit);

    template<typename... T>
    std::vector<double> getPosition(T&&... unit) {
        return getPosition({std::forward<T>(unit)...});
    }

    /**
     * Returns a string that represents the axes.
     * @return A string that represents the axes.
     */
    std::string toString();

    /**
     * Axes of the group.
     */
    std::vector<Axis> getAxes() const;

protected:
    std::vector<Axis> _axes;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
