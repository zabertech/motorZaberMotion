#include "zaber/motion/ascii/connection.h"
#include "zaberConnectionManager.h"
#include <catch.hpp>
#include <memory>

namespace ze = zaber::epics;
namespace zml = zaber::motion;

void resetConnectionStatics() {
    zml::ascii::Connection::openSerialPortMock = [](const std::string &) { return zml::ascii::Connection(); };
    zml::ascii::Connection::openTcpMock = [](const std::string &, int) { return zml::ascii::Connection(); };
}

TEST_CASE("zaberConnectionManager test", "[unit]") {
    ze::zaberConnectionManager &manager = ze::zaberConnectionManager::singleton();
    manager.removeAllConnections();
    resetConnectionStatics();

    SECTION("tryGetConnection tcp -- 1-token url without port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://localhost");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "localhost");
        REQUIRE(actualPort == DEFAULT_PORT);
    }

    SECTION("tryGetConnection tcp -- 1-token url with port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://localhost:12345");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "localhost");
        REQUIRE(actualPort == 12345);
    }


    SECTION("tryGetConnection tcp -- invalid port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        CHECK_THROWS_WITH(manager.tryGetConnection("tcp://localhost:1234x"), "Invalid TCP address: tcp://localhost:1234x");
    }

    SECTION("tryGetConnection tcp -- url without port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://my-super.domain.com");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "my-super.domain.com");
        REQUIRE(actualPort == DEFAULT_PORT);
    }

    SECTION("tryGetConnection tcp -- url with port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://my-super.domain.com:12345");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "my-super.domain.com");
        REQUIRE(actualPort == 12345);
    }

    SECTION("tryGetConnection tcp -- ipv4 address no port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://168.192.10.50");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "168.192.10.50");
        REQUIRE(actualPort == DEFAULT_PORT);
    }

    SECTION("tryGetConnection tcp -- ipv4 address with port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://168.192.10.50:12345");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "168.192.10.50");
        REQUIRE(actualPort == 12345);
    }

    SECTION("tryGetConnection tcp -- ipv6 address no port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
        REQUIRE(actualPort == DEFAULT_PORT);
    }

    SECTION("tryGetConnection tcp -- ipv6 address with port") {
        bool openSerialPortCalled = false;
        std::string actualPortName = "";
        int actualPort = 0;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &port, int portNumber) { actualPortName = port; actualPort = portNumber; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://[2001:0db8:85a3:0000:0000:8a2e:0370:7334]:12345");

        REQUIRE(openSerialPortCalled == false);
        REQUIRE(actualPortName == "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
        REQUIRE(actualPort == 12345);
    }

    SECTION("tryGetConnection tcp -- invalid url no port") {
        bool openSerialPortCalled = false;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };

        CHECK_THROWS_WITH(manager.tryGetConnection("tcp://my_invalid.domain.com"),
            "Invalid TCP address: tcp://my_invalid.domain.com");
    }

    SECTION("tryGetConnection tcp -- invalid url with port") {
        bool openSerialPortCalled = false;
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &) { openSerialPortCalled = true; return zml::ascii::Connection(); };

        CHECK_THROWS_WITH(manager.tryGetConnection("tcp://my_invalid.domain.com:12345"),
            "Invalid TCP address: tcp://my_invalid.domain.com:12345");
    }

    SECTION("tryGetConnection serial - windows") {
        bool openTcpCalled = false;
        std::string actualPortName = "";
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &port) { actualPortName = port; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &, int) { openTcpCalled = true; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("serial://COM1");

        REQUIRE(openTcpCalled == false);
        REQUIRE(actualPortName == "COM1");
    }

    SECTION("tryGetConnection serial - linux") {
        bool openTcpCalled = false;
        std::string actualPortName = "";
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &port) { actualPortName = port; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &, int) { openTcpCalled = true; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("serial:///dev/ttyS0");

        REQUIRE(openTcpCalled == false);
        REQUIRE(actualPortName == "/dev/ttyS0");
    }

    SECTION("tryGetConnection serial - darwin") {
        bool openTcpCalled = false;
        std::string actualPortName = "";
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &port) { actualPortName = port; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &, int) { openTcpCalled = true; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("serial:///dev/tty.usbserial-AB0PG6A5");

        REQUIRE(openTcpCalled == false);
        REQUIRE(actualPortName == "/dev/tty.usbserial-AB0PG6A5");
    }

    SECTION("tryGetConnection -- invalid port type") {
        bool openTcpCalled = false;
        std::string actualPortName = "";
        zml::ascii::Connection::openSerialPortMock =
            [&](const std::string &port) { actualPortName = port; return zml::ascii::Connection(); };
        zml::ascii::Connection::openTcpMock =
            [&](const std::string &, int) { openTcpCalled = true; return zml::ascii::Connection(); };

        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("/dev/oid");

        REQUIRE(openTcpCalled == false);
        REQUIRE(actualPortName == "/dev/oid");
    }

    SECTION("tryGetConnection -- same port returns same connection") {
        std::shared_ptr<zml::ascii::Connection> connection1 = manager.tryGetConnection("tcp://localhost");
        std::shared_ptr<zml::ascii::Connection> connection2 = manager.tryGetConnection("tcp://localhost");

        REQUIRE(connection1 == connection2);
    }

    SECTION("removeConnection -- removes with same interface id") {
        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://localhost");
        connection->interfaceId = 1;
        REQUIRE(manager.tryGetConnection("tcp://localhost") == connection);

        manager.removeConnection("tcp://localhost", 1);
        REQUIRE(manager.tryGetConnection("tcp://localhost") != connection);
    }

    SECTION("removeConnection -- does not remove with different interface id") {
        std::shared_ptr<zml::ascii::Connection> connection = manager.tryGetConnection("tcp://localhost");
        connection->interfaceId = 1;
        REQUIRE(manager.tryGetConnection("tcp://localhost") == connection);

        manager.removeConnection("tcp://localhost", 2);
        REQUIRE(manager.tryGetConnection("tcp://localhost") == connection);
    }

    SECTION("removePrefix") {
        REQUIRE(ze::zaberConnectionManager::removePrefix("tcp://", "tcp://localhost") == "localhost");
        REQUIRE(ze::zaberConnectionManager::removePrefix("tcp://", "tcp://localhost:12345") == "localhost:12345");
        REQUIRE(ze::zaberConnectionManager::removePrefix("serial://", "serial://COM1") == "COM1");
    }
}


