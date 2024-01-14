#pragma once

#include "Socket.hpp"
#include <cstdint>

namespace opendbus{

class TcpServer: public Socket {
private:
    int32_t listen_number_;

public:
    TcpServer(DBUS_DOMAIN domain, DBUS_SOCK sock, const std::string& address, int32_t listen_number)
    : Socket (domain, sock, address), listen_number_(listen_number) {
    }

    void bind() override {
        bindAddress();
        listenAddress(listen_number_);
        getEventLoopPtr()->setServerFlag(true);
        getEventLoopPtr()->start();
    }


    void socketSetup() override {
        // int optval = 1; // enable TCP_QUICKACK
        // setsockopt(getSocketFd(), IPPROTO_TCP, TCP_QUICKACK, (char *)&optval, sizeof(int));
        // optval = 1; // disenable TCP_CORK
        // setsockopt(getSocketFd(), IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
        // optval = 1; // disenable Nagle
        // setsockopt(getSocketFd(), IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof(int));
    }

private:
    void onRead(int32_t session_id) override {
        std::string data;
        if (read(session_id, data, 0) > 0 && read_cb_) {
            read_cb_(session_id, data);
        }
    }

    void onOnline(int32_t session_id) override {
        if (connect_cb_) {
            connect_cb_(session_id);
        }
    }

    void onOffline(int32_t session_id) override {
        if (disconnect_cb_) {
            disconnect_cb_(session_id);
        }
    }
};

} /* namespace opendbus */