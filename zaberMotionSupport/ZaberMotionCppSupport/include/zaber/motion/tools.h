// ===== THIS FILE IS GENERATED FROM A TEMPLATE ===== //
// ============== DO NOT EDIT DIRECTLY ============== //

#pragma once

#include <string>
#include <vector>


namespace zaber {
namespace motion {


/**
 * Class providing various utility functions.
 */
class Tools {
public:
    /**
     * Lists all serial ports on the computer.
     * @return Array of serial port names.
     */
    static std::vector<std::string> listSerialPorts();

    /**
     * Returns path of message router named pipe on Windows
     * or file path of unix domain socket on UNIX.
     * @return Path of message router's named pipe or unix domain socket.
     */
    static std::string getMessageRouterPipePath();

    /**
     * Returns the path for communicating with a local device database service.
     * This will be a named pipe on Windows and the file path of a unix domain socket on UNIX.
     * @return Path of database service's named pipe or unix domain socket.
     */
    static std::string getDbServicePipePath();

protected:
};


}  // Namespace motion
}  // Namespace zaber
