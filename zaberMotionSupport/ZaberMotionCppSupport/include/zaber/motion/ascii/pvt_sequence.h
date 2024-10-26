// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/dto/ascii/pvt_mode.h"
#include "zaber/motion/dto/ascii/pvt_axis_definition.h"
#include "zaber/motion/ascii/device.h"
#include "zaber/motion/ascii/pvt_io.h"
#include "zaber/motion/dto/ascii/digital_output_action.h"
#include <string>
#include <vector>
#include <optional>


namespace zaber {
namespace motion {

class Measurement;

namespace ascii {

/* Forward Declarations */
class PvtBuffer;


/**
 * A handle for a PVT sequence with this number on the device.
 * PVT sequences provide a way execute or store trajectory
 * consisting of points with defined position, velocity, and time.
 * PVT sequence methods append actions to a queue which executes
 * or stores actions in a first in, first out order.
 */
class PvtSequence {
public:
  struct WaitUntilIdleOptions {
    // Determines whether to throw error when fault is observed.
    bool throwErrorOnFault {true};
  };

    PvtSequence(Device device, int pvtId);

    PvtSequence();

    /**
     * Setup the PVT sequence to control the specified axes and to queue actions on the device.
     * Allows use of lockstep axes in a PVT sequence.
     * @param pvtAxes Definition of the PVT sequence axes.
     */
    void setupLiveComposite(std::initializer_list<PvtAxisDefinition> pvtAxes);

    template<typename... T>
    void setupLiveComposite(T&&... pvtAxes) {
        return setupLiveComposite({std::forward<T>(pvtAxes)...});
    }

    /**
     * Setup the PVT sequence to control the specified axes and to queue actions on the device.
     * @param axes Numbers of physical axes to setup the PVT sequence on.
     */
    void setupLive(std::initializer_list<int> axes);

    template<typename... T>
    void setupLive(T&&... axes) {
        return setupLive({std::forward<T>(axes)...});
    }

    /**
     * Setup the PVT sequence to use the specified axes and queue actions into a PVT buffer.
     * Allows use of lockstep axes in a PVT sequence.
     * @param pvtBuffer The PVT buffer to queue actions in.
     * @param pvtAxes Definition of the PVT sequence axes.
     */
    void setupStoreComposite(const PvtBuffer& pvtBuffer, std::initializer_list<PvtAxisDefinition> pvtAxes);

    template<typename... T>
    void setupStoreComposite(const PvtBuffer& pvtBuffer, T&&... pvtAxes) {
        return setupStoreComposite(pvtBuffer, {std::forward<T>(pvtAxes)...});
    }

    /**
     * Setup the PVT sequence to use the specified axes and queue actions into a PVT buffer.
     * @param pvtBuffer The PVT buffer to queue actions in.
     * @param axes Numbers of physical axes to setup the PVT sequence on.
     */
    void setupStore(const PvtBuffer& pvtBuffer, std::initializer_list<int> axes);

    template<typename... T>
    void setupStore(const PvtBuffer& pvtBuffer, T&&... axes) {
        return setupStore(pvtBuffer, {std::forward<T>(axes)...});
    }

    /**
     * Append the actions in a PVT buffer to the sequence's queue.
     * @param pvtBuffer The PVT buffer to call.
     */
    void call(const PvtBuffer& pvtBuffer);

    /**
     * Queues a point with absolute coordinates in the PVT sequence.
     * If some or all velocities are not provided, the sequence calculates the velocities
     * from surrounding points using finite difference.
     * The last point of the sequence must have defined velocity (likely zero).
     * @param positions Positions for the axes to move through, relative to their home positions.
     * @param velocities The axes velocities at the given point.
     * Specify an empty array or null for specific axes to make the sequence calculate the velocity.
     * @param time The duration between the previous point in the sequence and this one.
     */
    void point(const std::vector<Measurement>& positions, const std::vector<std::optional<Measurement>>& velocities, const Measurement& time);

    /**
     * Queues a point with coordinates relative to the previous point in the PVT sequence.
     * If some or all velocities are not provided, the sequence calculates the velocities
     * from surrounding points using finite difference.
     * The last point of the sequence must have defined velocity (likely zero).
     * @param positions Positions for the axes to move through, relative to the previous point.
     * @param velocities The axes velocities at the given point.
     * Specify an empty array or null for specific axes to make the sequence calculate the velocity.
     * @param time The duration between the previous point in the sequence and this one.
     */
    void pointRelative(const std::vector<Measurement>& positions, const std::vector<std::optional<Measurement>>& velocities, const Measurement& time);

    /**
     * Waits until the live PVT sequence executes all queued actions.
     * @param throwErrorOnFault Determines whether to throw error when fault is observed.
     */
    void waitUntilIdle(bool throwErrorOnFault = true);

    /**
     * Waits until the live PVT sequence executes all queued actions.
     * @param options A struct of type WaitUntilIdleOptions. It has the following members:
     * * `throwErrorOnFault`: Determines whether to throw error when fault is observed.
     */
    void waitUntilIdle(const PvtSequence::WaitUntilIdleOptions& options);

    /**
     * Cork the front of the PVT sequences's action queue, blocking execution.
     * Execution resumes upon uncorking the queue, or when the number of queued actions reaches its limit.
     * Corking eliminates discontinuities in motion due to subsequent PVT commands reaching the device late.
     * You can only cork an idle live PVT sequence.
     */
    void cork();

    /**
     * Uncork the front of the queue, unblocking command execution.
     * You can only uncork an idle live PVT sequence that is corked.
     */
    void uncork();

    /**
     * Returns a boolean value indicating whether the live PVT sequence is executing a queued action.
     * @return True if the PVT sequence is executing a queued action.
     */
    bool isBusy();

    /**
     * Returns a string which represents the PVT sequence.
     * @return String which represents the PVT sequence.
     */
    std::string toString();

    /**
     * Disables the PVT sequence.
     * If the PVT sequence is not setup, this command does nothing.
     * Once disabled, the PVT sequence will no longer accept PVT commands.
     * The PVT sequence will process the rest of the commands in the queue until it is empty.
     */
    void disable();

    /**
     * Sends a generic ASCII command to the PVT sequence.
     * Keeps resending the command while the device rejects with AGAIN reason.
     * @param command Command and its parameters.
     */
    void genericCommand(const std::string& command);

    /**
     * Sends a batch of generic ASCII commands to the PVT sequence.
     * Keeps resending command while the device rejects with AGAIN reason.
     * The batch is atomic in terms of thread safety.
     * @param batch Array of commands.
     */
    void genericCommandBatch(const std::vector<std::string>& batch);

    /**
     * Queries the PVT sequence status from the device
     * and returns boolean indicating whether the PVT sequence is disabled.
     * Useful to determine if execution was interrupted by other movements.
     * @return True if the PVT sequence is disabled.
     */
    bool checkDisabled();

    /**
     * Makes the PVT sequence throw PvtDiscontinuityException when it encounters discontinuities (ND warning flag).
     */
    void treatDiscontinuitiesAsError();

    /**
     * Prevents PvtDiscontinuityException as a result of expected discontinuity when resuming the sequence.
     */
    void ignoreCurrentDiscontinuity();

    /**
     * Deprecated: Use PvtSequence.Io.SetDigitalOutput instead.
     *
     * Sets value for the specified digital output channel.
     * @param channelNumber Channel number starting at 1.
     * @param value The type of action to perform on the channel.
     */
    void setDigitalOutput(int channelNumber, DigitalOutputAction value);

    /**
     * Deprecated: Use PvtSequence.Io.SetAllDigitalOutputs instead.
     *
     * Sets values for all digital output channels.
     * @param values The type of action to perform on the channel.
     */
    void setAllDigitalOutputs(const std::vector<DigitalOutputAction>& values);

    /**
     * Deprecated: Use PvtSequence.Io.SetAnalogOutput instead.
     *
     * Sets value for the specified analog output channel.
     * @param channelNumber Channel number starting at 1.
     * @param value Value to set the output channel voltage to.
     */
    void setAnalogOutput(int channelNumber, double value);

    /**
     * Deprecated: Use PvtSequence.Io.SetAllAnalogOutputs instead.
     *
     * Sets values for all analog output channels.
     * @param values Voltage values to set the output channels to.
     */
    void setAllAnalogOutputs(const std::vector<double>& values);

    /**
     * Device that controls this PVT sequence.
     */
    Device getDevice() const;

    /**
     * The number that identifies the PVT sequence on the device.
     */
    int getPvtId() const;

    /**
     * Current mode of the PVT sequence.
     */
    PvtMode getMode() const;

    /**
     * An array of axes definitions the PVT sequence is set up to control.
     */
    std::vector<PvtAxisDefinition> getAxes() const;

    /**
     * Gets an object that provides access to I/O for this sequence.
     */
    PvtIo getIo() const;

protected:
    /**
     * Gets the axes of the PVT sequence.
     * @return An array of axis numbers of the axes the PVT sequence is set up to control.
     */
    std::vector<PvtAxisDefinition> retrieveAxes() const;
    /**
     * Get the mode of the PVT sequence.
     * @return Mode of the PVT sequence.
     */
    PvtMode retrieveMode() const;
    Device _device;
    int _pvtId;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
