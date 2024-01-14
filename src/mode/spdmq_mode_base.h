/*
*   Copyright 2024 billy_yan billyany@163.com
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*/

#pragma once

#include "spdmq_def.h"
// #include "EventFactory.hpp"
// #include "SocketFactory.hpp"

namespace speed::mq {

// enum class DBUS_NET_PROTOCOL : uint8_t {
//     TCP = 0,
//     UDP = 1,
//     IPC = 2,
// };

// const std::map<std::string, DBUS_NET_PROTOCOL> gDBusNetProtocolMap = {
//     {"tcp", DBUS_NET_PROTOCOL::TCP},
//     {"udp", DBUS_NET_PROTOCOL::UDP},
//     {"ipc", DBUS_NET_PROTOCOL::IPC},
// };

class spdmq_mode_base {
public:
    spdmq_ctx_t& ctx_;
//     DBUS_NET_PROTOCOL protocol_;
//     std::shared_ptr<Socket> socket_ptr_;
//     opendbus::DBUS_SOCKET_MODE socket_mode_;
//     std::shared_ptr<EventLoop> event_loop_ptr_;
//     std::function<void(const DBusData& dbusData)> read_cb_; // 读取事件回调
//     std::function<void(int32_t)> connect_cb_; // 连接事件回调
//     std::function<void(int32_t)> disconnect_cb_; // 断开事件回调

public:
    spdmq_mode_base(spdmq_ctx& ctx) : ctx_(ctx) {
        // event_loop_ptr_ = EVT_CREATE(opendbus::DBUS_EVENT_MODE::EPOLL_ET, ctx_.event_num);
    }

    virtual ~spdmq_mode_base() {}

    // virtual int32_t send(DBusData& dbusData) { return 0; }

    // virtual int32_t recv(DBusData& dbusData, uint32_t time_out) { return 0; }

    // virtual void onRead(int32_t session_id, const std::string& data) = 0;
    
    // virtual void onOnline(int32_t session_id) = 0;

    // virtual void onOffline(int32_t session_id) = 0;

    int32_t bind(const std::string& address) {
        // socket_ptr_ = SOCK_CREATE(socket_mode_, address, ctx_.event_num);
        // socket_ptr_->setEventLoopPtr(event_loop_ptr_);
        // socket_ptr_->createSocket();
        // socket_ptr_->socketSetup();
        // socket_ptr_->resolveAddress();
        // socket_ptr_->setReadCallback(std::bind(&spdmq_mode_base::onRead, this, std::placeholders::_1, std::placeholders::_2));
        // socket_ptr_->setOnlineCallback(std::bind(&spdmq_mode_base::onOnline, this, std::placeholders::_1));
        // socket_ptr_->setOfflineCallback(std::bind(&spdmq_mode_base::onOffline, this, std::placeholders::_1));
        // socket_ptr_->bind();
        // event_loop_ptr_->backgroundRun();
        // return 0;
    }

    int32_t connect(const std::string& address) {
        // socket_ptr_ = SOCK_CREATE(socket_mode_, address, ctx_.event_num);
        // socket_ptr_->setEventLoopPtr(event_loop_ptr_);
        // socket_ptr_->createSocket();
        // socket_ptr_->socketSetup();
        // socket_ptr_->resolveAddress();
        // socket_ptr_->setReadCallback(std::bind(&spdmq_mode_base::onRead, this, std::placeholders::_1, std::placeholders::_2));
        // socket_ptr_->setOnlineCallback(std::bind(&spdmq_mode_base::onOnline, this, std::placeholders::_1));
        // socket_ptr_->setOfflineCallback(std::bind(&spdmq_mode_base::onOffline, this, std::placeholders::_1));
        // socket_ptr_->connect();
        // event_loop_ptr_->backgroundRun();
        // return 0;
    }

    // void setReadCallback(std::function<void(const DBusData& dbusData)> cb) {
    //     read_cb_ = cb;
    // }

    // void setOnlineCallback(std::function<void(int32_t)> cb) {
    //     connect_cb_ = cb;
    // }

    // void setOfflineCallback(std::function<void(int32_t)> cb) {
    //     disconnect_cb_ = cb;
    // }

};

}