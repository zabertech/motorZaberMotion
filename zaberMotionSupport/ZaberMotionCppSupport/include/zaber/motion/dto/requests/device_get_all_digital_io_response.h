// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace requests {

class DeviceGetAllDigitalIOResponse: public Serializable {
public:
    std::vector<bool> values;

    DeviceGetAllDigitalIOResponse();

    DeviceGetAllDigitalIOResponse(
        std::vector<bool> p_values
    );

    bool operator==(const DeviceGetAllDigitalIOResponse& other) const;

    bool operator!=(const DeviceGetAllDigitalIOResponse& other) const {
        return !(*this == other);
    }

    std::vector<bool> const& getValues() const;
    void setValues(std::vector<bool> p_values);

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
