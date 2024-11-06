#ifndef CONNECTION_MOCK_H
#define CONNECTION_MOCK_H

#include <memory>
#include <functional>

namespace zaber {
namespace motion{

namespace exceptions {
class MotionLibException {
public:
    MotionLibException() {}
    virtual ~MotionLibException() {}
};
} // exceptions

namespace ascii {

class Connection {
public:
    Connection() {}
    ~Connection() {}
    void identify() { }
    int getInterfaceId() { return -1; }
    void setDisconnectedCallback(const std::function<void(const std::shared_ptr<zaber::motion::exceptions::MotionLibException>&)>& callback) {
        (void)callback;
    }
    static Connection openSerialPort(const std::string& port) {
        (void)port;
        return Connection();
    }
    static Connection openNetworkShare(const std::string& hostnameOrIp) {
        (void)hostnameOrIp;
        return Connection();
    }
};

} // ascii
} // motion
} // zaber

#endif
