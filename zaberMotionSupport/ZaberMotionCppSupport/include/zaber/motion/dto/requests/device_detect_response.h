// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace requests {

class DeviceDetectResponse: public Serializable {
public:
    std::vector<int> devices;

    DeviceDetectResponse();

    DeviceDetectResponse(
        std::vector<int> p_devices
    );

    bool operator==(const DeviceDetectResponse& other) const;

    bool operator!=(const DeviceDetectResponse& other) const {
        return !(*this == other);
    }

    std::vector<int> const& getDevices() const;
    void setDevices(std::vector<int> p_devices);

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
