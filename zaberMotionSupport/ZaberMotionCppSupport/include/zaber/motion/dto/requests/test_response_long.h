// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <vector>
#include <string>

#include "zaber/motion/dto/serializable.h"

namespace zaber {
namespace motion {
namespace requests {

class TestResponseLong: public Serializable {
public:
    std::vector<std::string> dataPong;

    TestResponseLong();

    TestResponseLong(
        std::vector<std::string> p_dataPong
    );

    bool operator==(const TestResponseLong& other) const;

    bool operator!=(const TestResponseLong& other) const {
        return !(*this == other);
    }

    std::vector<std::string> const& getDataPong() const;
    void setDataPong(std::vector<std::string> p_dataPong);

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
