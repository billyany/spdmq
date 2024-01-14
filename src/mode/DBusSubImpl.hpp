#pragma once

#include "DBusBase.hpp"
#include "DBusFunc.hpp"
#include "DBusLogger.hpp"
#include "DBusQueue.hpp"

namespace opendbus {

class DBusSubImpl : public DBusBase {
private:
    DBusQueue<DBusData> dataQueue_;
    std::mutex lock_;
    std::condition_variable cv_;

public:
    DBusSubImpl(const DBusCtx& ctx, DBUS_NET_PROTOCOL protocol) : DBusBase(ctx, protocol) {
        socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_TCP_CLI;
        if (protocol == DBUS_NET_PROTOCOL::TCP) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_TCP_CLI;
        }
        else if (protocol == DBUS_NET_PROTOCOL::UDP) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_UDP_CLI;
        }
        else if (protocol == DBUS_NET_PROTOCOL::IPC) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::UDS_TCP_CLI;
        }
    }

    int32_t recv(DBusData& dbusData, uint32_t time_out) override {
        std::unique_lock<std::mutex> lk(lock_);
        if (dataQueue_.empty() && 0 == time_out) {
            cv_.wait(lk, [this] { return !dataQueue_.empty(); });
        }
        else if (dataQueue_.empty() && !cv_.wait_for(lk, std::chrono::microseconds(time_out), [this] { return !dataQueue_.empty(); })) {
            return -1; // timeout
        }

        dbusData = dataQueue_.pop();

        return 0;
    }

    void onRead(int32_t session_id, const std::string& data) override {
        DBusData dbusData = deserializeDBusData(data);
        dbusData.recv_time_stamp = nowUsTimestamp();
        dbusData.time_cost = dbusData.recv_time_stamp - dbusData.send_time_stamp;
        if (read_cb_) {
            read_cb_(dbusData);
            return;
        }
        dataQueue_.push(std::move(dbusData));
        cv_.notify_all();
    }

    void onOnline(int32_t session_id) override {
        for (auto topic : ctx_.topics) {
            DBusData dbusData;
            dbusData.msg_type = DBUS_MESSAGE_TYPE::DBUS_TOPIC;
            dbusData.topic = topic;
            socket_ptr_->write(serializeDBusData(dbusData));
        }
    }

    void onOffline(int32_t session_id) override {

    }
};

}