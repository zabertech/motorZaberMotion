// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/dto/ascii/pvt_sequence_data.h"

namespace zaber {
namespace motion {
namespace requests {

class PvtCsvRequest: public Serializable {
public:
    ascii::PvtSequenceData sequenceData;
    std::string path;

    PvtCsvRequest();

    PvtCsvRequest(
        ascii::PvtSequenceData p_sequenceData,
        std::string p_path
    );

    bool operator==(const PvtCsvRequest& other) const;

    bool operator!=(const PvtCsvRequest& other) const {
        return !(*this == other);
    }

    ascii::PvtSequenceData const& getSequenceData() const;
    void setSequenceData(ascii::PvtSequenceData p_sequenceData);

    std::string const& getPath() const;
    void setPath(std::string p_path);

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
