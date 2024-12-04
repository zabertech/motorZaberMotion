// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include "zaber/motion/ascii/connection.h"
#include "zaber/motion/ascii/device.h"
#include "zaber/motion/gateway/call.h"
#include "zaber/motion/dto/requests/includes.h"

namespace zaber {
namespace motion {
namespace ascii {

/* Forward Declarations */
class PvtBuffer;


/**
 * Represents a PVT buffer with this number on a device.
 * A PVT buffer is a place to store a queue of PVT actions.
 */
class PvtBuffer {
public:
    PvtBuffer(Device device, int bufferId);

    /**
     * Gets the buffer contents as an array of strings.
     * @return A string array containing all the PVT commands stored in the buffer.
     */
    std::vector<std::string> getContent();

    /**
     * Erases the contents of the buffer.
     * This method fails if there is a PVT sequence writing to the buffer.
     */
    void erase();

    /**
     * The Device this buffer exists on.
     */
    Device getDevice() const;

    /**
     * The number identifying the buffer on the device.
     */
    int getBufferId() const;

protected:
    Device _device;
    int _bufferId;
};


}  // Namespace ascii
}  // Namespace motion
}  // Namespace zaber
