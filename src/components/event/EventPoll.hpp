#pragma once

#include <cstdint>
#include <cstring>
#include <thread>
#include "EventLoop.hpp"
#include "DBusLogger.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace opendbus {

class EventPoll : public EventLoop {
private:
    int32_t mode_;

public:
    EventPoll(int32_t mode, int32_t event_num) : EventLoop(event_num), mode_(mode) {
        fd_ = epoll_create1(0);
        if (fd_ == -1) {
            std::stringstream ss;
            ss << "creating epoll instance failed, reason: " << std::strerror(errno);
            LOGE(ss.str());
            throw std::runtime_error(ss.str());
        }
        std::thread(&EventPoll::EventPollLoop, this).detach();
    }

    void start() override {
        if (event_start_flag_) {
            LOGI("Do not start repeatedly");
            return;
        }

        event_start_flag_ = true;
        struct epoll_event event;
        event.events = mode_;
        event.data.fd = socket_fd_;
        if (epoll_ctl(fd_, EPOLL_CTL_ADD, socket_fd_, &event) == -1) {
            std::stringstream ss;
            ss << "adding socket to epoll failed, reason: " << std::strerror(errno);
            LOGE(ss.str());
            throw std::runtime_error(ss.str());
        }
    }

    void stop() override {
        if (!event_start_flag_) {
            LOGI("Do not stop repeatedly");
            return;
        }

        event_start_flag_ = false;
        epoll_ctl(fd_, EPOLL_CTL_DEL, socket_fd_, nullptr);
        close(socket_fd_);
    }

    void destroy() override {
        if (event_destroy_flag_) {
            LOGI("Do not destroy repeatedly");
            return;
        }

        event_destroy_flag_ = true;
    }



    int32_t read(int32_t session_id, std::string& data, uint32_t time_out = 0) override {

        struct timeval tv;
        tv.tv_sec = 0;  // 设置超时秒数为 0
        tv.tv_usec = time_out * 1000; // 设置超时微秒数也为 0

        // setsockopt 超时设置
        setsockopt(session_id, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        // 读取数据头
        int32_t data_len = 0;
        {
            size_t total_bytes_received = 0;

            // 分段读取数据体
            while (total_bytes_received < 4) { 
                auto bytes_received = recv(session_id, &data_len + total_bytes_received, 4 - total_bytes_received, 0);
                LOGD("data_len:", data_len, ", bytes_received:", bytes_received);

                if (bytes_received <= 0) {
                    if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                        return -1;
                    }
                    return readFailedDeal(session_id);
                }
                total_bytes_received += bytes_received;
            }
        }
        LOGD("data_len:", data_len);

        // 读取数据体
        data.resize(data_len, 0);
        size_t total_bytes_received = 0;

        // 分段读取数据体
        while (total_bytes_received < data_len) {
            auto bytes_received = recv(session_id, &data.data()[total_bytes_received], data_len - total_bytes_received, 0);
            LOGD("data_len:", data_len, ", bytes_received:", bytes_received);

            if (bytes_received <= 0) {
                if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                    return -1;
                }
                return readFailedDeal(session_id);
            }
            total_bytes_received += bytes_received;
        }

        return total_bytes_received;
        
    }

    int32_t write(int32_t session_id, const std::string& data) override {
        std::stringstream ss;
        int32_t data_len = data.size();
        ss.write((const char*)&data_len, sizeof data_len);
        ss << data;
        return send(session_id, ss.str().data(), ss.str().size(), MSG_DONTROUTE | MSG_NOSIGNAL);
    }

private:
    int32_t readFailedDeal(int32_t session_id) {
        epoll_ctl(fd_, EPOLL_CTL_DEL, session_id, nullptr);
        client_fd_set_.erase(session_id);
        close(session_id);
        event_map_queue_[DBUS_EVENT_PRIORITY::NORMAL].push({session_id, DBUS_EVENT::DISCONNECT});
        cv_.notify_all();
        return -1;
    }

    void EventPollLoop() {
        std::shared_ptr<epoll_event> eventsPtr(new epoll_event[event_num_](), [] (epoll_event* events) { delete [] events; });
        while (true) {
            if (event_destroy_flag_) {
                break;
            }

            epoll_event* events = eventsPtr.get();
            int num_events = epoll_wait(fd_, eventsPtr.get(), event_num_, 1000);
            
            if (num_events == -1) {
                std::stringstream ss;
                ss << "epoll_wait failed, reason: " << std::strerror(errno);
                LOGE(ss.str());
                throw std::runtime_error(ss.str());
            }

            // 遍历事件
            for (int i = 0; i < num_events; ++i) {
                if (-1 == events[i].data.fd) continue;

                // 有新连接
                if (events[i].data.fd == socket_fd_ && server_flag_) {
                    sockaddr_in client_address;
                    socklen_t client_address_size = sizeof(client_address);
                    int client_fd = accept(socket_fd_, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);
                    if (client_fd == -1) {
                        LOGW("Error accepting connection.");
                        continue;
                    }
                    LOGD("Connection from ", inet_ntoa(client_address.sin_addr), ":", ntohs(client_address.sin_port));

                    // 将新连接的套接字添加到 epoll 实例中
                    events[i].events = mode_;
                    events[i].data.fd = client_fd;
                    epoll_ctl(fd_, EPOLL_CTL_ADD, client_fd, &events[i]);
                    client_fd_set_.insert(client_fd);
                    event_map_queue_[DBUS_EVENT_PRIORITY::NORMAL].push({client_fd, DBUS_EVENT::CONNECT});
                    cv_.notify_all();
                    continue;
                }

                int32_t client_fd = events[i].data.fd;
                event_map_queue_[DBUS_EVENT_PRIORITY::NORMAL].push({client_fd, DBUS_EVENT::READ});
                cv_.notify_all();
            }
        }
    }
};

} /* opendbus*/