// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <optional>
#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/servo_tuning_paramset.h"
#include "zaber/motion/dto/ascii/servo_tuning_param.h"

namespace zaber {
namespace motion {
namespace requests {

class SetSimpleTuning: public Serializable {
public:
    int interfaceId {0};
    int device {0};
    int axis {0};
    ascii::ServoTuningParamset paramset {0};
    std::optional<double> carriageMass;
    double loadMass {0.0};
    std::vector<ascii::ServoTuningParam> tuningParams;

    SetSimpleTuning();

    SetSimpleTuning(
        int p_interfaceId,
        int p_device,
        int p_axis,
        ascii::ServoTuningParamset p_paramset,
        std::optional<double> p_carriageMass,
        double p_loadMass,
        std::vector<ascii::ServoTuningParam> p_tuningParams
    );

    SetSimpleTuning(
        int p_interfaceId,
        int p_device,
        int p_axis,
        ascii::ServoTuningParamset p_paramset,
        double p_loadMass,
        std::vector<ascii::ServoTuningParam> p_tuningParams
    );

    bool operator==(const SetSimpleTuning& other) const;

    bool operator!=(const SetSimpleTuning& other) const {
        return !(*this == other);
    }

    int getInterfaceId() const;
    void setInterfaceId(int p_interfaceId);

    int getDevice() const;
    void setDevice(int p_device);

    int getAxis() const;
    void setAxis(int p_axis);

    ascii::ServoTuningParamset const& getParamset() const;
    void setParamset(ascii::ServoTuningParamset p_paramset);

    std::optional<double> getCarriageMass() const;
    void setCarriageMass(std::optional<double> p_carriageMass);

    double getLoadMass() const;
    void setLoadMass(double p_loadMass);

    std::vector<ascii::ServoTuningParam> const& getTuningParams() const;
    void setTuningParams(std::vector<ascii::ServoTuningParam> p_tuningParams);

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

} // namespace requests
} // namespace motion
} // namespace zaber
