// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <optional>
#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/servo_tuning_param.h"

namespace zaber {
namespace motion {
namespace ascii {

/**
 * The masses and parameters last used by simple tuning.
 */
class SimpleTuning: public Serializable {
public:
    bool isUsed {false};
    bool isSet {false};
    std::optional<double> carriageMass;
    std::optional<double> motorInertia;
    double loadMass {0.0};
    std::vector<ServoTuningParam> tuningParams;

    SimpleTuning();

    SimpleTuning(
        bool p_isUsed,
        bool p_isSet,
        std::optional<double> p_carriageMass,
        std::optional<double> p_motorInertia,
        double p_loadMass,
        std::vector<ServoTuningParam> p_tuningParams
    );

    SimpleTuning(
        bool p_isUsed,
        bool p_isSet,
        double p_loadMass,
        std::vector<ServoTuningParam> p_tuningParams
    );

    bool operator==(const SimpleTuning& other) const;

    bool operator!=(const SimpleTuning& other) const {
        return !(*this == other);
    }

    /**
     * Whether the tuning returned is currently in use by this paramset,
     * or if it has been overwritten by a later change.
     */
    bool getIsUsed() const;
    void setIsUsed(bool p_isUsed);

    /**
     * If this paramset has been tuned using the simple tuning method, whether or not it's currently in use.
     */
    bool getIsSet() const;
    void setIsSet(bool p_isSet);

    /**
     * The mass of the carriage in kg.
     */
    std::optional<double> getCarriageMass() const;
    void setCarriageMass(std::optional<double> p_carriageMass);

    /**
     * The mass of the carriage in kg.
     */
    std::optional<double> getMotorInertia() const;
    void setMotorInertia(std::optional<double> p_motorInertia);

    /**
     * The mass of the load in kg, excluding the mass of the carriage.
     */
    double getLoadMass() const;
    void setLoadMass(double p_loadMass);

    /**
     * The parameters used by simple tuning.
     */
    std::vector<ServoTuningParam> const& getTuningParams() const;
    void setTuningParams(std::vector<ServoTuningParam> p_tuningParams);

    /**
     * Convert object to human-readable string format
     */
    std::string toString() const;

#ifdef ZML_SERIALIZATION_PUBLIC
public:
#else
private:
#endif

    std::string toByteArray() const override;
    void populateFromByteArray(const std::string& buffer) override;

};

} // namespace ascii
} // namespace motion
} // namespace zaber
