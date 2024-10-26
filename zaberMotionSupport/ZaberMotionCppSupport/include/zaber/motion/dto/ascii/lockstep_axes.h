// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace ascii {

/**
 * The axis numbers of a lockstep group.
 */
class LockstepAxes: public Serializable {
public:
    int axis1 {0};
    int axis2 {0};
    int axis3 {0};
    int axis4 {0};

    LockstepAxes();

    LockstepAxes(
        int p_axis1,
        int p_axis2,
        int p_axis3,
        int p_axis4
    );

    bool operator==(const LockstepAxes& other) const;

    bool operator!=(const LockstepAxes& other) const {
        return !(*this == other);
    }

    /**
     * The axis number used to set the first axis.
     */
    int getAxis1() const;
    void setAxis1(int p_axis1);

    /**
     * The axis number used to set the second axis.
     */
    int getAxis2() const;
    void setAxis2(int p_axis2);

    /**
     * The axis number used to set the third axis.
     */
    int getAxis3() const;
    void setAxis3(int p_axis3);

    /**
     * The axis number used to set the fourth axis.
     */
    int getAxis4() const;
    void setAxis4(int p_axis4);

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
