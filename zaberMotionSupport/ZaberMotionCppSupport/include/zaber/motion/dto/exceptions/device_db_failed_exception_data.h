// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace exceptions {

/**
 * Contains additional data for a DeviceDbFailedException.
 */
class DeviceDbFailedExceptionData: public Serializable {
public:
    std::string code;

    DeviceDbFailedExceptionData();

    DeviceDbFailedExceptionData(
        std::string p_code
    );

    bool operator==(const DeviceDbFailedExceptionData& other) const;

    bool operator!=(const DeviceDbFailedExceptionData& other) const {
        return !(*this == other);
    }

    /**
     * Code describing type of the error.
     */
    std::string const& getCode() const;
    void setCode(std::string p_code);

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

} // namespace exceptions
} // namespace motion
} // namespace zaber
