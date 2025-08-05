// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <optional>
#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/measurement_sequence.h"
#include "zaber/motion/dto/ascii/optional_measurement_sequence.h"

namespace zaber {
namespace motion {
namespace requests {

class PvtGenerateVelocitiesRequest: public Serializable {
public:
    std::vector<ascii::MeasurementSequence> positions;
    std::vector<ascii::OptionalMeasurementSequence> velocities;
    ascii::MeasurementSequence times;
    bool timesRelative {false};

    PvtGenerateVelocitiesRequest();

    PvtGenerateVelocitiesRequest(
        std::vector<ascii::MeasurementSequence> p_positions,
        std::vector<ascii::OptionalMeasurementSequence> p_velocities,
        ascii::MeasurementSequence p_times,
        bool p_timesRelative
    );

    PvtGenerateVelocitiesRequest(
        std::vector<ascii::MeasurementSequence> p_positions,
        ascii::MeasurementSequence p_times,
        bool p_timesRelative
    );

    bool operator==(const PvtGenerateVelocitiesRequest& other) const;

    bool operator!=(const PvtGenerateVelocitiesRequest& other) const {
        return !(*this == other);
    }

    std::vector<ascii::MeasurementSequence> const& getPositions() const;
    void setPositions(std::vector<ascii::MeasurementSequence> p_positions);

    std::vector<ascii::OptionalMeasurementSequence> const& getVelocities() const;
    void setVelocities(std::vector<ascii::OptionalMeasurementSequence> p_velocities);

    ascii::MeasurementSequence const& getTimes() const;
    void setTimes(ascii::MeasurementSequence p_times);

    bool getTimesRelative() const;
    void setTimesRelative(bool p_timesRelative);

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
