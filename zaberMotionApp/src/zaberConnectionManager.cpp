#include "zaberConnectionManager.h"

#include <regex>
#include <iostream>

zaberConnectionManager& zaberConnectionManager::singleton() {
    static zaberConnectionManager instance;
    return instance;
}

std::shared_ptr<zml::ascii::Connection> zaberConnectionManager::tryGetConnection(const std::string& port) {
    std::shared_ptr<zml::ascii::Connection> connection;
    if (_connections.find(port) != _connections.end()) {
        if (connection = _connections[port].lock()) {
            return connection;
        }
    }
    
    if (port.find("tcp://") == 0) {
        std::string portSanitized = zaberConnectionManager::removePrefix("tcp://", port);
        std::regex tcpAddressRegex(R"((\[([a-fA-F0-9:]+)\]|([a-zA-Z0-9.-]+))(:(\d{1,5}))?)");
        std::smatch match;
        if (std::regex_match(port, match, tcpAddressRegex)) {
            std::string hostnameOrIp = match[2].matched ? match[2].str() : match[3].str();
            std::string port = match[5].str();
            std::cout << "hostnameOrIp: " << hostnameOrIp << " port: " << port << std::endl;
            if (!port.empty()) {
                int portNumber = std::stoi(port); (void)portNumber;
                // connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openTcp(hostnameOrIp, portNumber));
                connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openNetworkShare(hostnameOrIp));
            } else {
                // connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openTcp(hostnameOrIp));
                connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openNetworkShare(hostnameOrIp));
            }
        } else {
            connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openSerialPort(port));
        }
    } else if (port.find("serial://") == 0) {
        std::string portSanitized = zaberConnectionManager::removePrefix("serial://", port);
        connection = std::make_shared<zml::ascii::Connection>(zml::ascii::Connection::openSerialPort(portSanitized));
    } else {
        std::cerr << "Unknown connection type: " << port << std::endl;
        std::cerr << "Connection type must be prefixed with tcp:// or serial://" << std::endl;
    }


    int id = connection->getInterfaceId();
    connection->setDisconnectedCallback([this, port, id](const std::shared_ptr<zml::exceptions::MotionLibException>&) {
        removeConnection(port, id);
    });
    _connections[port] = connection;
    return connection;
}

void zaberConnectionManager::removeConnection(const std::string& port, int interfaceId) {
    auto it = _connections.find(port);
    if (it == _connections.end() || interfaceId == -1) {
        return;
    }

    if (std::shared_ptr<zml::ascii::Connection> connection = it->second.lock()) {
        if (connection->getInterfaceId() != -1) {
            return;
        }
    }
    _connections.erase(port);
}

std::string zaberConnectionManager::removePrefix(const std::string& prefix, const std::string& str) {
    if (str.find(prefix) == 0) {
        return str.substr(prefix.length());
    }
    return str;
}
