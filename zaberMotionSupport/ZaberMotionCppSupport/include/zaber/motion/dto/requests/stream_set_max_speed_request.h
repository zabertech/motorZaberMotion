// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

#include "zaber/motion/dto/serializable.h"
#include "zaber/motion/units.h"

namespace zaber {
namespace motion {
namespace requests {

class StreamSetMaxSpeedRequest: public Serializable {
public:
    int interfaceId {0};
    int device {0};
    int streamId {0};
    bool pvt {false};
    double maxSpeed {0.0};
    Units unit {Units::NATIVE};

    StreamSetMaxSpeedRequest();

    StreamSetMaxSpeedRequest(
        int p_interfaceId,
        int p_device,
        int p_streamId,
        bool p_pvt,
        double p_maxSpeed,
        Units p_unit
    );

    bool operator==(const StreamSetMaxSpeedRequest& other) const;

    bool operator!=(const StreamSetMaxSpeedRequest& other) const {
        return !(*this == other);
    }

    int getInterfaceId() const;
    void setInterfaceId(int p_interfaceId);

    int getDevice() const;
    void setDevice(int p_device);

    int getStreamId() const;
    void setStreamId(int p_streamId);

    bool getPvt() const;
    void setPvt(bool p_pvt);

    double getMaxSpeed() const;
    void setMaxSpeed(double p_maxSpeed);

    Units getUnit() const;
    void setUnit(Units p_unit);

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
