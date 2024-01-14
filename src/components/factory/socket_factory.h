#pragma once

#include "TcpServer.hpp"
#include "TcpClient.hpp"
#include "UdsServer.hpp"
#include "UdsClient.hpp"
#include <memory>

#define SOCK_CREATE opendbus::SocketFactory::getInstance()->createSocket

namespace opendbus {

enum class DBUS_SOCKET_MODE : uint8_t {
    IPV4_TCP_SER = 0,
    IPV4_TCP_CLI = 1,
    IPV4_UDP_SER = 2,
    IPV4_UDP_CLI = 3,
    IPV6_TCP_SER = 4,
    IPV6_TCP_CLI = 5,
    IPV6_UDP_SER = 6,
    IPV6_UDP_CLI = 7,
    UDS_TCP_SER = 8,
    UDS_TCP_CLI = 9,
    UDS_UDP_SER = 10,
    UDS_UDP_CLI = 11,
};

class SocketFactory {
public:
    static SocketFactory* getInstance() {
        static SocketFactory impl;
        return &impl;
    }

    std::shared_ptr<Socket> createSocket(DBUS_SOCKET_MODE mode, const std::string& address, int32_t number) {
        std::shared_ptr<Socket> socket_ptr;
        switch (mode) {
            case DBUS_SOCKET_MODE::IPV4_TCP_SER:
                socket_ptr = std::make_shared<TcpServer>(opendbus::DBUS_DOMAIN::DBUS_DOMAIN_IPV4, opendbus::DBUS_SOCK::DBUS_SOCK_TCP, address, number);
                break;
            case DBUS_SOCKET_MODE::IPV4_TCP_CLI:
                socket_ptr = std::make_shared<TcpClient>(opendbus::DBUS_DOMAIN::DBUS_DOMAIN_IPV4, opendbus::DBUS_SOCK::DBUS_SOCK_TCP, address, number);
                break;
            case DBUS_SOCKET_MODE::IPV4_UDP_SER:
            case DBUS_SOCKET_MODE::IPV4_UDP_CLI:
            case DBUS_SOCKET_MODE::IPV6_TCP_SER:
            case DBUS_SOCKET_MODE::IPV6_TCP_CLI:
            case DBUS_SOCKET_MODE::IPV6_UDP_SER:
            case DBUS_SOCKET_MODE::IPV6_UDP_CLI:
            case DBUS_SOCKET_MODE::UDS_TCP_SER:
                socket_ptr = std::make_shared<UdsServer>(opendbus::DBUS_DOMAIN::DBUS_DOMAIN_UDS, opendbus::DBUS_SOCK::DBUS_SOCK_TCP, address, number);
                break;
            case DBUS_SOCKET_MODE::UDS_TCP_CLI:
                socket_ptr = std::make_shared<UdsClient>(opendbus::DBUS_DOMAIN::DBUS_DOMAIN_UDS, opendbus::DBUS_SOCK::DBUS_SOCK_TCP, address, number);
                break;
            case DBUS_SOCKET_MODE::UDS_UDP_SER:
            case DBUS_SOCKET_MODE::UDS_UDP_CLI:
                break;
        }
        return socket_ptr;
    }
};

} /* opendbus */
