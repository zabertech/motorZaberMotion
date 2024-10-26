// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace requests {

class StreamSetAllAnalogOutputsRequest: public Serializable {
public:
    int interfaceId {0};
    int device {0};
    int streamId {0};
    bool pvt {false};
    std::vector<double> values;

    StreamSetAllAnalogOutputsRequest();

    StreamSetAllAnalogOutputsRequest(
        int p_interfaceId,
        int p_device,
        int p_streamId,
        bool p_pvt,
        std::vector<double> p_values
    );

    bool operator==(const StreamSetAllAnalogOutputsRequest& other) const;

    bool operator!=(const StreamSetAllAnalogOutputsRequest& other) const {
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

    std::vector<double> const& getValues() const;
    void setValues(std::vector<double> p_values);

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
