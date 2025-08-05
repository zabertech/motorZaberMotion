// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <optional>
#include <vector>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/pvt_sequence_data.h"

namespace zaber {
namespace motion {
namespace requests {

class PvtSaveCsvRequest: public Serializable {
public:
    ascii::PvtSequenceData sequenceData;
    std::string path;
    std::vector<std::string> dimensionNames;

    PvtSaveCsvRequest();

    PvtSaveCsvRequest(
        ascii::PvtSequenceData p_sequenceData,
        std::string p_path,
        std::vector<std::string> p_dimensionNames
    );

    PvtSaveCsvRequest(
        ascii::PvtSequenceData p_sequenceData,
        std::string p_path
    );

    bool operator==(const PvtSaveCsvRequest& other) const;

    bool operator!=(const PvtSaveCsvRequest& other) const {
        return !(*this == other);
    }

    ascii::PvtSequenceData const& getSequenceData() const;
    void setSequenceData(ascii::PvtSequenceData p_sequenceData);

    std::string const& getPath() const;
    void setPath(std::string p_path);

    std::vector<std::string> const& getDimensionNames() const;
    void setDimensionNames(std::vector<std::string> p_dimensionNames);

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
