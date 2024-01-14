#pragma once

#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <set>
#include <map>
#include <mutex>
#include <thread>
#include <cstdint>
#include <functional>
#include <condition_variable>

#include "DBusQueue.hpp"
#include "DBusLogger.hpp"

namespace opendbus {

enum class DBUS_EVENT_MODE : uint8_t {
    EPOLL_LT = 0, // epoll 水平触发
    EPOLL_ET = 1, // epoll 边缘触发
};

enum class DBUS_EVENT_PRIORITY : uint8_t {
    NORMAL = 0,
    URGENT = 1,
};

enum class DBUS_EVENT : uint8_t {
    READ = 0,
    WRITE = 1,
    CONNECT = 2,
    DISCONNECT = 3,
};

class EventLoop {
public:
    int32_t fd_; // 事件处理对象 fd
    int32_t socket_fd_; // 监听对象 fd
    int32_t event_num_; // 监听的事件数
    bool server_flag_; // 服务端 flag
    bool event_start_flag_; // 事件开始 flag
    bool event_run_flag_; // 事件运行 flag
    bool event_destroy_flag_;  // 事件停止 flag
    std::mutex lock_; // 事件锁
    std::condition_variable cv_; // 事件条件锁
    std::map<DBUS_EVENT_PRIORITY, DBusQueue<std::pair<int32_t, DBUS_EVENT>>> event_map_queue_; // 事件队列表，包含正常事件和紧急事件
    std::function<void(int32_t)> event_read_cb_; // 读取事件回调
    std::function<void(int32_t)> event_connect_cb_; // 连接事件回调
    std::function<void(int32_t)> event_disconnect_cb_; // 断开事件回调
    std::set<int32_t> client_fd_set_;

public:
    EventLoop(int32_t event_num) {
        fd_ = -1;
        server_flag_ = false;
        event_start_flag_ = false;
        event_run_flag_ = false;
        event_destroy_flag_ = false;
        socket_fd_ = -1;
        event_num_ = event_num;
    }

    virtual ~EventLoop() {}

    virtual void start() = 0;

    virtual void stop() = 0;

    virtual void destroy() = 0;

    virtual int32_t read(int32_t session_id, std::string& data, uint32_t time_out = 0) = 0;

    virtual int32_t write(int32_t session_id, const std::string& data) = 0;

    void setServerFlag(bool server_flag) {
        server_flag_ = server_flag;
    }

    void setSocketFd(int32_t server_fd) {
        socket_fd_ = server_fd;
    }

    void setEventNum(int32_t event_num) {
        event_num_ = event_num;
    }

    void foregroundRun() {
        if (event_run_flag_) { 
            LOGI("Do not run repeatedly");
            return;
        }

        event_run_flag_ = true;
        run();
    }

    void backgroundRun() {
        if (event_run_flag_) { 
            LOGI("Do not run repeatedly");
            return;
        }

        event_run_flag_ = true;
        std::thread(&EventLoop::run, this).detach();
    }

    DBusQueue<std::pair<int32_t, DBUS_EVENT>>& getNormalQueue() {
        return event_map_queue_[DBUS_EVENT_PRIORITY::NORMAL];
    }

    DBusQueue<std::pair<int32_t, DBUS_EVENT>>& getUrgentQueue() {
        return event_map_queue_[DBUS_EVENT_PRIORITY::URGENT];
    }

    void setReadCallback(std::function<void(int32_t)> cb) {
        event_read_cb_ = cb;
    }

    void setOnlineCallback(std::function<void(int32_t)> cb) {
        event_connect_cb_ = cb;
    }

    void setOfflineCallback(std::function<void(int32_t)> cb) {
        event_disconnect_cb_ = cb;
    }

private:
    void run() {
        while (true) {
            if (event_destroy_flag_) break;
            {
                std::unique_lock<std::mutex> lk(lock_);
                cv_.wait(lk, [this] { return !getNormalQueue().empty() || !getUrgentQueue().empty(); });
            }

            consumeQueue(getUrgentQueue());
            consumeQueue(getNormalQueue());
        }
    }

    void consumeQueue(DBusQueue<std::pair<int32_t, DBUS_EVENT>>& queue) {
        while (!queue.empty()) {
            auto event = queue.pop();
            if (DBUS_EVENT::READ == event.second) {
                if (event_read_cb_) {
                    event_read_cb_(event.first);
                }
                continue;
            }
            if (DBUS_EVENT::CONNECT == event.second) {
                if (event_connect_cb_) {
                    event_connect_cb_(event.first);
                }
                continue;
            }
            if (DBUS_EVENT::DISCONNECT == event.second) {
                if (event_disconnect_cb_) {
                    event_disconnect_cb_(event.first);
                }
                continue;
            }
        }
    }
};

}