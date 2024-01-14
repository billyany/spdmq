#pragma once

#include "DBusBase.hpp"
#include "DBusFunc.hpp"
#include "DBusLogger.hpp"
#include "DBusSpinLock.hpp"
#include <cstdint>

namespace opendbus {

class DBusPubImpl : public DBusBase {
private:
    std::map<std::string, std::set<int32_t>> subscribe_table_; // subscription topic table
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;

public:
    DBusPubImpl(const DBusCtx& ctx, DBUS_NET_PROTOCOL protocol) : DBusBase(ctx, protocol) {
        socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_TCP_SER;
        if (protocol == DBUS_NET_PROTOCOL::TCP) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_TCP_SER;
        }
        else if (protocol == DBUS_NET_PROTOCOL::UDP) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::IPV4_UDP_SER;
        }
        else if (protocol == DBUS_NET_PROTOCOL::IPC) {
            socket_mode_ = opendbus::DBUS_SOCKET_MODE::UDS_TCP_SER;
        }
    }

    int32_t send(DBusData& dbusData) override {
        dbusData.send_time_stamp = nowUsTimestamp();
        auto data = serializeDBusData(dbusData);
        DBusSpinLock<std::atomic_flag> lk(lock_);
        for (auto session_id : subscribe_table_[dbusData.topic]) {
            socket_ptr_->write(session_id, data);
        }
        return 0;
    }

private:
    void onRead(int32_t session_id, const std::string& data) override {
        DBusData dbusData = deserializeDBusData(data);
        DBusSpinLock<std::atomic_flag> lk(lock_);
        if (dbusData.msg_type == DBUS_MESSAGE_TYPE::DBUS_TOPIC) {
            subscribe_table_[dbusData.topic].insert(session_id);
        }
        else if (dbusData.msg_type == DBUS_MESSAGE_TYPE::DBUS_HEARTBEAT) {
            // TODO
        }
    }
    
    void onOnline(int32_t session_id) override {
        // TODO
        // LOGD("onOnline session_id:", session_id);
        // subscribe_table_["opendbus"].insert(session_id);
    }

    void onOffline(int32_t session_id) override {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        for (auto& session_id_set : subscribe_table_) {
            session_id_set.second.erase(session_id);
        }
    }
};

}