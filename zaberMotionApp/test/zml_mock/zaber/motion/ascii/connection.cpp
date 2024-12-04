#include "connection.h"

using namespace zaber::motion::ascii;

std::function<Connection(const std::string &)> Connection::openSerialPortMock = [](const std::string &) { return Connection(); };
std::function<Connection(const std::string &, int)> Connection::openTcpMock = [](const std::string &, int) { return Connection(); };
