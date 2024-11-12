#include "zaberConnectionManager.h"

#include <regex>
#include <iostream>

using namespace zaber::epics;

zaberConnectionManager& zaberConnectionManager::singleton() {
    static zaberConnectionManager instance;
    return instance;
}

std::shared_ptr<zml::ascii::Connection> zaberConnectionManager::tryGetConnection(const std::string& port) {
    std::shared_ptr<zml::ascii::Connection> connection;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_connections.find(port) != _connections.end()) {
            if ((connection = _connections[port].lock())) {
                return connection;
            }
        }
    }

    if (port.find("tcp://") == 0) {
        std::string portSanitized = zaberConnectionManager::removePrefix("tcp://", port);
        std::regex tcpAddressRegex(R"((\[([a-fA-F0-9:]+)\]|([a-zA-Z0-9.-]+))(:(\d{1,5}))?)");
        std::smatch match;
        if (std::regex_match(portSanitized, match, tcpAddressRegex)) {
            std::string hostnameOrIp = match[2].matched ? match[2].str() : match[3].str();
            std::string port = match[5].str();
            if (!port.empty()) {
                int portNumber = std::stoi(port); (void)portNumber;
                // connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openTcp(hostnameOrIp, portNumber));
                connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openNetworkShare(hostnameOrIp));
            } else {
                // connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openTcp(hostnameOrIp));
                connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openNetworkShare(hostnameOrIp));
            }
        } else {
            throw std::runtime_error("Invalid TCP address: " + port);
        }
    } else if (port.find("serial://") == 0) {
        std::string portSanitized = zaberConnectionManager::removePrefix("serial://", port);
        connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openSerialPort(portSanitized));
    } else {
        // default to serial port if no prefix is provided
        connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openSerialPort(port));
    }

    int id = connection->getInterfaceId();
    connection->setDisconnectedCallback([this, port, id](const std::shared_ptr<zml::exceptions::MotionLibException>&) {
        removeConnection(port, id);
    });

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _connections[port] = connection;
    }

    return connection;
}

void zaberConnectionManager::removeConnection(const std::string& port, int interfaceId) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _connections.find(port);
    if (it == _connections.end() || interfaceId == -1) {
        return;
    }

    if (std::shared_ptr<zml::ascii::Connection> connection = it->second.lock()) {
        if (connection->getInterfaceId() != interfaceId) {
            return;
        }
    }
    _connections.erase(port);
}

void zaberConnectionManager::removeAllConnections() {
    std::lock_guard<std::mutex> lock(_mutex);
    _connections.clear();
}

std::string zaberConnectionManager::removePrefix(const std::string& prefix, const std::string& str) {
    if (str.find(prefix) == 0) {
        return str.substr(prefix.length());
    }
    return str;
}
