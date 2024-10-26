// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/units.h"
#include <string>
#include <vector>

#include "zaber/motion/dto/ascii/servo_tuning_paramset.h"
#include "zaber/motion/dto/ascii/paramset_info.h"
#include "zaber/motion/dto/ascii/pid_tuning.h"
#include "zaber/motion/dto/ascii/servo_tuning_param.h"
#include "zaber/motion/dto/ascii/simple_tuning_param_definition.h"
#include "zaber/motion/ascii/axis.h"
#include "zaber/motion/dto/ascii/simple_tuning.h"


namespace zaber {
namespace motion {
namespace ascii {


/**
 * Exposes the capabilities to inspect and edit an axis' servo tuning.
 * Requires at least Firmware 6.25 or 7.00.
 */
class ServoTuner {
public:
  struct SetSimpleTuningOptions {
    // The mass of the carriage in kg. If this value is not set the default carriage mass is used.
    std::optional<double> carriageMass {};
  };

  struct IsUsingSimpleTuningOptions {
    // The mass of the carriage in kg. If this value is not set the default carriage mass is used.
    std::optional<double> carriageMass {};
  };

    /**
     * Creates instance of ServoTuner for the given axis.
     */
    ServoTuner(Axis axis);

    /**
     * Get the paramset that this device uses by default when it starts up.
     * @return The paramset used when the device restarts.
     */
    ServoTuningParamset getStartupParamset();

    /**
     * Set the paramset that this device uses by default when it starts up.
     * @param paramset The paramset to use at startup.
     */
    void setStartupParamset(ServoTuningParamset paramset);

    /**
     * Load the values from one paramset into another.
     * @param toParamset The paramset to load into.
     * @param fromParamset The paramset to load from.
     */
    void loadParamset(ServoTuningParamset toParamset, ServoTuningParamset fromParamset);

    /**
     * Get the full set of tuning parameters used by the firmware driving this axis.
     * @param paramset The paramset to get tuning for.
     * @return The raw representation of the current tuning.
     */
    ParamsetInfo getTuning(ServoTuningParamset paramset);

    /**
     * Set individual tuning parameters.
     * Only use this method if you have a strong understanding of Zaber specific tuning parameters.
     * @param paramset The paramset to set tuning of.
     * @param tuningParams The params to set.
     */
    void setTuning(ServoTuningParamset paramset, const std::vector<ServoTuningParam>& tuningParams);

    /**
     * Sets the tuning of a paramset using the PID method.
     * @param paramset The paramset to get tuning for.
     * @param p The proportional gain. Must be in units of N/m.
     * @param i The integral gain. Must be in units of N/m⋅s.
     * @param d The derivative gain. Must be in units of N⋅s/m.
     * @param fc The cutoff frequency. Must be in units of Hz.
     * @return The PID representation of the current tuning after your changes have been applied.
     */
    PidTuning setPidTuning(ServoTuningParamset paramset, double p, double i, double d, double fc);

    /**
     * Gets the PID representation of this paramset's servo tuning.
     * @param paramset The paramset to get tuning for.
     * @return The PID representation of the current tuning.
     */
    PidTuning getPidTuning(ServoTuningParamset paramset);

    /**
     * Gets the parameters that are required to tune this device.
     * @return The tuning parameters.
     */
    std::vector<SimpleTuningParamDefinition> getSimpleTuningParamDefinitions();

    /**
     * Set the tuning of this device using the simple input method.
     * @param paramset The paramset to set tuning for.
     * @param tuningParams The params used to tune this device.
     * To get what parameters are expected, call GetSimpleTuningParamList.
     * All values must be between 0 and 1.
     * @param loadMass The mass loaded on the stage (excluding the mass of the carriage itself) in kg.
     * @param carriageMass The mass of the carriage in kg. If this value is not set the default carriage mass is used.
     */
    void setSimpleTuning(ServoTuningParamset paramset, const std::vector<ServoTuningParam>& tuningParams, double loadMass, const std::optional<double>& carriageMass = {});

    /**
     * Set the tuning of this device using the simple input method.
     * @param paramset The paramset to set tuning for.
     * @param tuningParams The params used to tune this device.
     * To get what parameters are expected, call GetSimpleTuningParamList.
     * All values must be between 0 and 1.
     * @param loadMass The mass loaded on the stage (excluding the mass of the carriage itself) in kg.
     * @param options A struct of type SetSimpleTuningOptions. It has the following members:
     * * `carriageMass`: The mass of the carriage in kg. If this value is not set the default carriage mass is used.
     */
    void setSimpleTuning(ServoTuningParamset paramset, const std::vector<ServoTuningParam>& tuningParams, double loadMass, const ServoTuner::SetSimpleTuningOptions& options);

    /**
     * Get the simple tuning parameters for this device.
     * @param paramset The paramset to get tuning for.
     * @return The simple tuning parameters.
     */
    SimpleTuning getSimpleTuning(ServoTuningParamset paramset);

    /**
     * Deprecated: Use GetSimpleTuning instead.
     *
     * Checks if the provided simple tuning is being stored by this paramset.
     * @param paramset The paramset to set tuning for.
     * @param tuningParams The params used to tune this device.
     * To get what parameters are expected, call GetSimpleTuningParamList.
     * All values must be between 0 and 1.
     * @param loadMass The mass loaded on the stage (excluding the mass of the carriage itself) in kg.
     * @param carriageMass The mass of the carriage in kg. If this value is not set the default carriage mass is used.
     * @return True if the provided simple tuning is currently stored in this paramset.
     */
    bool isUsingSimpleTuning(ServoTuningParamset paramset, const std::vector<ServoTuningParam>& tuningParams, double loadMass, const std::optional<double>& carriageMass = {});

    /**
     * Deprecated: Use GetSimpleTuning instead.
     *
     * Checks if the provided simple tuning is being stored by this paramset.
     * @param paramset The paramset to set tuning for.
     * @param tuningParams The params used to tune this device.
     * To get what parameters are expected, call GetSimpleTuningParamList.
     * All values must be between 0 and 1.
     * @param loadMass The mass loaded on the stage (excluding the mass of the carriage itself) in kg.
     * @param options A struct of type IsUsingSimpleTuningOptions. It has the following members:
     * * `carriageMass`: The mass of the carriage in kg. If this value is not set the default carriage mass is used.
     * @return True if the provided simple tuning is currently stored in this paramset.
     */
    bool isUsingSimpleTuning(ServoTuningParamset paramset, const std::vector<ServoTuningParam>& tuningParams, double loadMass, const ServoTuner::IsUsingSimpleTuningOptions& options);

    /**
     * The axis that will be tuned.
     */
    Axis getAxis() const;

protected:
    Axis _axis;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
