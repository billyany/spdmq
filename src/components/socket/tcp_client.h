#pragma once

#include "Socket.hpp"
#include "DBusDef.h"

namespace opendbus{

class TcpClient: public Socket {
public:
    TcpClient(DBUS_DOMAIN domain, DBUS_SOCK sock, const std::string& address, int32_t heart_interval)
    : Socket (domain, sock, address) {
        heart_interval_ = heart_interval;
        heart_start_flag_ = false;
    }

    void connect() override {
        connectAddress();
    }

    void startHeart() override {
        // prevent duplicate calls
        if (heart_start_flag_) {
            LOGI("Do not startHeart repeatedly");
            return;
        }
        heart_start_flag_ = true;

        // 启动心跳线程
        std::thread([this] {
            while (true) {
                if (!heart_start_flag_) break;
                DBusData dbusData;
                dbusData.msg_type = DBUS_MESSAGE_TYPE::DBUS_HEARTBEAT;
                int ret = Socket::write(getSocketFd(), serializeDBusData(dbusData));
                if (-1 == ret) {
                    onOffline(getSocketFd());
                    break;
                }
                usleep(heart_interval_ * 1000);
            }
        }).detach();
    }

    void stopHeart() override {
        heart_start_flag_ = false;
        // 触发重连机制
        createSocket();
        connectAddress();
    }

    int32_t read(std::string& data, uint32_t time_out) override { 
        return Socket::read(getSocketFd(), data, time_out);; 
    }

    int32_t write(const std::string& data) override {
        return Socket::write(getSocketFd(), data);
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
        while (Socket::read(session_id, data, 0) > 0 && read_cb_) {
            read_cb_(session_id, data);
        }
    }

    void onOnline(int32_t session_id) override {
        getEventLoopPtr()->start();

        startHeart();

        if (connect_cb_) {
            connect_cb_(session_id);
        }
    }

    void onOffline(int32_t session_id) override {
        getEventLoopPtr()->stop();
        stopHeart();

        if (disconnect_cb_) {
            disconnect_cb_(session_id);
        }
    }
};

} /* namespace opendbus */