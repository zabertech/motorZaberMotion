// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/measurement_sequence.h"

namespace zaber {
namespace motion {
namespace requests {

class PvtGeneratePositionsRequest: public Serializable {
public:
    std::vector<ascii::MeasurementSequence> velocities;
    ascii::MeasurementSequence times;
    bool timesRelative {false};

    PvtGeneratePositionsRequest();

    PvtGeneratePositionsRequest(
        std::vector<ascii::MeasurementSequence> p_velocities,
        ascii::MeasurementSequence p_times,
        bool p_timesRelative
    );

    bool operator==(const PvtGeneratePositionsRequest& other) const;

    bool operator!=(const PvtGeneratePositionsRequest& other) const {
        return !(*this == other);
    }

    std::vector<ascii::MeasurementSequence> const& getVelocities() const;
    void setVelocities(std::vector<ascii::MeasurementSequence> p_velocities);

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
