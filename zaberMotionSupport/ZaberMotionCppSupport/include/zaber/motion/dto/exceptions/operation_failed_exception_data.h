// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace exceptions {

/**
 * Contains additional data for OperationFailedException.
 */
class OperationFailedExceptionData: public Serializable {
public:
    std::vector<std::string> warnings;
    std::string reason;
    int device {0};
    int axis {0};

    OperationFailedExceptionData();

    OperationFailedExceptionData(
        std::vector<std::string> p_warnings,
        std::string p_reason,
        int p_device,
        int p_axis
    );

    bool operator==(const OperationFailedExceptionData& other) const;

    bool operator!=(const OperationFailedExceptionData& other) const {
        return !(*this == other);
    }

    /**
     * The full list of warnings.
     */
    std::vector<std::string> const& getWarnings() const;
    void setWarnings(std::vector<std::string> p_warnings);

    /**
     * The reason for the Exception.
     */
    std::string const& getReason() const;
    void setReason(std::string p_reason);

    /**
     * The address of the device that attempted the failed operation.
     */
    int getDevice() const;
    void setDevice(int p_device);

    /**
     * The number of the axis that attempted the failed operation.
     */
    int getAxis() const;
    void setAxis(int p_axis);

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
