// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/measurement_sequence.h"

namespace zaber {
namespace motion {
namespace ascii {

/**
 * Data representing a sequence of pvt points with defined positions, velocities and times.
 */
class PvtSequenceData: public Serializable {
public:
    std::vector<MeasurementSequence> positions;
    std::vector<MeasurementSequence> velocities;
    MeasurementSequence times;

    PvtSequenceData();

    PvtSequenceData(
        std::vector<MeasurementSequence> p_positions,
        std::vector<MeasurementSequence> p_velocities,
        MeasurementSequence p_times
    );

    bool operator==(const PvtSequenceData& other) const;

    bool operator!=(const PvtSequenceData& other) const {
        return !(*this == other);
    }

    /**
     * Pvt sequence positions for each axis.
     */
    std::vector<MeasurementSequence> const& getPositions() const;
    void setPositions(std::vector<MeasurementSequence> p_positions);

    /**
     * Pvt velocities for each axis.
     */
    std::vector<MeasurementSequence> const& getVelocities() const;
    void setVelocities(std::vector<MeasurementSequence> p_velocities);

    /**
     * Relative times corresponding to points in PVT sequence.
     */
    MeasurementSequence const& getTimes() const;
    void setTimes(MeasurementSequence p_times);

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
