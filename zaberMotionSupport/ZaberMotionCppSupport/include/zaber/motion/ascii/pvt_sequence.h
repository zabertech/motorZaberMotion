// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <vector>
#include <optional>

#include "zaber/motion/ascii/device.h"
#include "zaber/motion/ascii/pvt_io.h"
#include "zaber/motion/dto/ascii/digital_output_action.h"
#include "zaber/motion/dto/ascii/pvt_axis_definition.h"
#include "zaber/motion/dto/ascii/pvt_mode.h"
#include "zaber/motion/dto/ascii/pvt_csv_data.h"
#include "zaber/motion/dto/measurement.h"
#include "zaber/motion/dto/ascii/measurement_sequence.h"
#include "zaber/motion/dto/ascii/optional_measurement_sequence.h"

namespace zaber {
namespace motion {
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
     * Queues points with absolute coordinates in the PVT sequence.
     * @param positions Per-axis sequences of positions.
     * @param velocities Per-axis sequences of velocities.
     * For velocities [v0, v1, ...] and positions [p0, p1, ...], v1 is the target velocity at point p1.
     * @param times Segment times from one point to another.
     * For times [t0, t1, ...] and positions [p0, p1, ...], t1 is the time it takes to move from p0 to p1.
     */
    void points(const std::vector<MeasurementSequence>& positions, const std::vector<MeasurementSequence>& velocities, const MeasurementSequence& times);

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
     * Queues points with coordinates relative to the previous point in the PVT sequence.
     * @param positions Per-axis sequences of positions.
     * @param velocities Per-axis sequences of velocities.
     * For velocities [v0, v1, ...] and positions [p0, p1, ...], v1 is the target velocity at point p1.
     * @param times Segment times from one point to another.
     * For times [t0, t1, ...] and positions [p0, p1, ...], t1 is the time it takes to move from p0 to p1.
     */
    void pointsRelative(const std::vector<MeasurementSequence>& positions, const std::vector<MeasurementSequence>& velocities, const MeasurementSequence& times);

    /**
     * Generates velocities for a sequence of positions and times, and (optionally) a partially defined sequence
     * of velocities. Note that if some velocities are defined, the solver will NOT modify them in any way.
     * If all velocities are defined, the solver will simply return the same velocities.
     * This function calculates velocities by enforcing that acceleration be continuous at each segment transition.
     *
     * Also note that if generating a path for multiple axes, the user must provide a position measurement sequence
     * per axis, And the values arrays for each sequence must be equal in length to each other and also to the
     * times sequence.
     * @param positions Positions for the axes to move through, relative to their home positions.
     * Each MeasurementSequence represents a sequence of positions along a particular dimension.
     * For example, a 2D path sequence would contain two MeasurementSequence objects,
     * one representing positions along X and one for those along Y.
     * @param times The relative or absolute time of each position in the PVT sequence.
     * @param velocities Optional velocities corresponding to each point in the position sequences.
     * @param timesRelative If true, the times sequence values are interpreted as relative. Otherwise,
     * they are interpreted as absolute. Note that the values of the returned time
     * sequence are ALWAYS relative. This is because the PVT sequence API expects
     * points to have relative times.
     * @return Object containing the generated PVT sequence. Note that returned time sequence is always relative.
     */
    static PvtSequenceData generateVelocities(const std::vector<MeasurementSequence>& positions, const MeasurementSequence& times, const std::vector<OptionalMeasurementSequence>& velocities = {}, bool timesRelative = true);

    /**
     * Generates positions for a sequence of velocities and times. This function calculates
     * positions by enforcing that acceleration be continuous at each segment transition.
     *
     * Note that if generating a path for multiple axes, the user must provide a
     * velocity measurement sequence per axis, and the values arrays for each sequence
     * must be equal in length to each other and also to the times sequence.
     * @param velocities The sequence of velocities for each axis.
     * Each MeasurementSequence represents a sequence of velocities along particular dimension.
     * @param times The relative or absolute time of each position in the PVT sequence.
     * @param timesRelative If true, the times sequence values are interpreted as relative. Otherwise,
     * they are interpreted as absolute. Note that the values of the returned time
     * sequence are ALWAYS relative. This is because the PVT sequence API expects
     * points to have relative times.
     * @return Object containing the generated PVT sequence. Note that returned time sequence is always relative.
     */
    static PvtSequenceData generatePositions(const std::vector<MeasurementSequence>& velocities, const MeasurementSequence& times, bool timesRelative = true);

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
    std::string toString() const;

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
     * Saves PvtSequenceData object as csv file.
     * Save format is compatible with Zaber Launcher PVT Editor App.
     *
     * Throws InvalidArgumentException if fields are undefined or inconsistent.
     * For example, position and velocity arrays must have the same dimensions.
     * Sequence lengths must be consistent for positions, velocities and times.
     * @param sequenceData The PVT sequence data to save.
     * @param path The path to save the file to.
     * @param dimensionNames Optional csv column names for each series.
     * If not provided, the default names will be used: Series 1, Series 2, etc..
     * Length of this array must be equal to number of dimensions in sequence data.
     */
    static void saveSequenceData(const PvtSequenceData& sequenceData, const std::string& path, const std::vector<std::string>& dimensionNames = {});

    /**
     * Load PVT Sequence data from CSV file.
     * The CSV data can include a header (recommended).
     * There are two possible header formats:
     *
     * 1. A time column with named position and velocity columns.
     * For example, "Time (ms),X Position (cm),X Velocity (cm/s),...".
     * In this case, position, velocity and time columns are all optional.
     * Also, order does not matter, but position and velocity names must be consistent.
     * This is our recommended CSV format.
     *
     * 2. A time column with alternating position and velocity columns.
     * For example, "Time (ms),Position (cm),Velocity (cm/s),...".
     * In this case, only the time column is optional and order does matter.
     *
     * Units must be wrapped in parens or square braces: ie. (µm/s), [µm/s].
     * Additionally, native units are the default if no units are specified.
     * If no header is included, then column order is assumed to be "T,P1,V1,P2,V2,...".
     * In this case the number of columns must be odd.
     * @param path The path from which the csv file will be loaded.
     * @return The PVT csv data loaded from the file.
     */
    static PvtCsvData loadSequenceData(const std::string& path);

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
