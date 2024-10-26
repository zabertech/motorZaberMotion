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
    std::optional<double> carriageMass;
    double loadMass {0.0};
    std::vector<ServoTuningParam> tuningParams;

    SimpleTuning();

    SimpleTuning(
        bool p_isUsed,
        std::optional<double> p_carriageMass,
        double p_loadMass,
        std::vector<ServoTuningParam> p_tuningParams
    );

    SimpleTuning(
        bool p_isUsed,
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
     * The mass of the carriage in kg.
     */
    std::optional<double> getCarriageMass() const;
    void setCarriageMass(std::optional<double> p_carriageMass);

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
