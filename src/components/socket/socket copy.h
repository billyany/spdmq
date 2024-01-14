#pragma once

#include "socket_fd.h"


#include <map>
#include <memory>
#include <thread>
#include <cstdint>
#include <sstream>
#include <stdexcept>

#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

// #include "DBusLogger.hpp"
// #include "EventLoop.hpp"
// #include "DBusFileLock.hpp"

namespace speed::mq {

enum class DBUS_DOMAIN : uint8_t {
    DBUS_DOMAIN_IPV4 = 0,
    DBUS_DOMAIN_IPV6 = 1,
    DBUS_DOMAIN_UDS = 2,
};

enum class DBUS_SOCK : uint8_t {
    DBUS_SOCK_TCP = 0,
    DBUS_SOCK_UDP = 1,
};

const std::map<DBUS_DOMAIN, int32_t> gDBusDomainMap = {
    {DBUS_DOMAIN::DBUS_DOMAIN_IPV4, AF_INET},
    {DBUS_DOMAIN::DBUS_DOMAIN_IPV6, AF_INET6},
    {DBUS_DOMAIN::DBUS_DOMAIN_UDS, AF_UNIX},
};

const std::map<DBUS_SOCK, int32_t> gDBusSockMap = {
    {DBUS_SOCK::DBUS_SOCK_TCP, SOCK_STREAM},
    {DBUS_SOCK::DBUS_SOCK_UDP, SOCK_DGRAM},
};

class socket {
public:
    int32_t set_nosigpipe (spdmq_fd_t fd);

public:
    int32_t socket_fd_;
    sockaddr_un sock_address_un_;
    sockaddr_in sock_address_ipv4_;
    sockaddr_in6 sock_address_ipv6_;
    DBUS_DOMAIN domain_;
    DBUS_SOCK sock_;
    std::string address_;
    std::string ip_;
    uint16_t port_;
    std::shared_ptr<DBusFileLock> file_lock_;
    

    bool connected_; // tcp mode client connect status
    int32_t heart_interval_;
    bool heart_start_flag_;

    std::shared_ptr<EventLoop> event_loop_ptr_;
    std::function<void(int32_t, const std::string&)> read_cb_; // 读取事件回调
    std::function<void(int32_t)> connect_cb_; // 连接事件回调
    std::function<void(int32_t)> disconnect_cb_; // 断开事件回调

public:
    virtual void bind() {}
    virtual void connect() {}
    virtual void socketSetup() {}
    virtual void startHeart() {}
    virtual void stopHeart() {}
    virtual int32_t read(std::string& data, uint32_t time_out) { return 0; }
    virtual int32_t write(const std::string& data) { return 0; }

    virtual void onRead(int32_t session_id) = 0;
    virtual void onOnline(int32_t session_id) = 0;
    virtual void onOffline(int32_t session_id) = 0;


public:
    socket(DBUS_DOMAIN domain, DBUS_SOCK sock, const std::string& address) :
        socket_fd_(-1),
        sock_address_un_(),
        sock_address_ipv4_(),
        domain_(domain),
        sock_(sock),
        address_(address),
        connected_(false) {

        // parse the address
        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_IPV4) {
            ip_ = address.substr(0, address.find(":"));
            port_ = std::atoi(address.substr(address.find(":") + 1).c_str());
        }
    }

    virtual ~socket() {
        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_UDS) {
            unlink(address_.c_str());
        }
    }

    void createSocket() {
        
        // create socket file descriptor
        closeSocketFd();
        socket_fd_ = socket(gDBusDomainMap.at(domain_), gDBusSockMap.at(sock_), 0);
        if (socket_fd_ == -1) { throwdException("creating socket failed"); }
        
        // add the socket_fd_ to the event loop
        getEventLoopPtr()->setSocketFd(socket_fd_);

        connected_ = false;
    }

    void resolveAddress() {
        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_IPV4) {
            sock_address_ipv4_.sin_family = gDBusDomainMap.at(domain_);
            inet_pton(gDBusDomainMap.at(domain_), ip_.c_str(), &(sock_address_ipv4_.sin_addr));
            sock_address_ipv4_.sin_port = htons(port_);
            LOGD("resolve ipv4 address, fd:", socket_fd_, ", ip:", ip_, ", port:", port_);
            return;
        }

        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_UDS) {
            sock_address_un_.sun_family = gDBusDomainMap.at(domain_);
            strncpy(sock_address_un_.sun_path, address_.data(), sizeof(sock_address_un_.sun_path) - 1);
            LOGD("resolve uds address, fd:", socket_fd_, "address:", address_);
            return;
        }
    }

    void bindAddress() {
        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_IPV4) {
            // enable address reuse
            int optval = 1; // 1 - enable, 0 - disenable
            if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
                closeSocketFd();
                throwdException("setting SO_REUSEADDR failed"); 
            }

            // bind socket_fd_ to address_
            if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&sock_address_ipv4_), sizeof(sock_address_ipv4_)) == -1) {
                closeSocketFd();
                throwdException("binding server socket failed");
            }
            return;
        }

        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_UDS) {

            try {
                LOGD("address_:", address_);
                std::string lock_address = address_ + ".lock";
                if (access(lock_address.c_str(), F_OK) == 0) {
                    file_lock_ = std::make_shared<DBusFileLock>(address_ + ".lock", true);
                    unlink(address_.c_str());
                }
                else {
                    file_lock_ = std::make_shared<DBusFileLock>(address_ + ".lock", true);
                }
            }
            catch (...) {
                throwdException("binding server socket failed, ipc file lock");
            }

            // bind socket_fd_ to address_
            if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&sock_address_un_), sizeof(sock_address_un_)) == -1) {
                closeSocketFd();
                throwdException("binding server socket failed");
            }
        }

    }

    void listenAddress(uint32_t listen_number) {
        if (listen(socket_fd_, listen_number) == -1) {
            closeSocketFd();
            throwdException("listening on server socket failed");
        }
    }

    void connectAddress() {
        // prevent duplicate calls
        if (connected_) return;

        std::thread([this] {
            while (true) {
                if (onConnect()) {
                    LOGD("connect failed: ", std::strerror(errno));
                    usleep(1000 * 1000);
                    continue;
                }
                connected_ = true;
                onOnline(socket_fd_);
                break;
            }
        }).detach();
    }

    int32_t read(int32_t session_id, std::string& data, uint32_t time_out) {
        return getEventLoopPtr()->read(session_id, data, time_out);
    }

    int32_t write(int32_t session_id, const std::string& data) {
        return getEventLoopPtr()->write(session_id, data);
    }

public:
    int32_t getSocketFd() {
        return socket_fd_;
    }

    std::shared_ptr<EventLoop> getEventLoopPtr() {
        return event_loop_ptr_;
    }

    void setEventLoopPtr(std::shared_ptr<EventLoop> event_loop_ptr) {
        event_loop_ptr_ = event_loop_ptr;
        getEventLoopPtr()->setReadCallback(std::bind(&socket::onRead, this, std::placeholders::_1));
        getEventLoopPtr()->setOnlineCallback(std::bind(&socket::onOnline, this, std::placeholders::_1));
        getEventLoopPtr()->setOfflineCallback(std::bind(&socket::onOffline, this, std::placeholders::_1));
    }

    void setReadCallback(std::function<void(int32_t, const std::string&)> cb) {
        read_cb_ = cb;
    }

    void setOnlineCallback(std::function<void(int32_t)> cb) {
        connect_cb_ = cb;
    }

    void setOfflineCallback(std::function<void(int32_t)> cb) {
        disconnect_cb_ = cb;
    }

private:
    void throwdException(const std::string& msg) {
        std::stringstream ss;
        ss << msg << ", reason: " << std::strerror(errno);
        LOGE(ss.str());
        throw std::runtime_error(ss.str());
    }

    int32_t onConnect() {
        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_IPV4) {
            return ::connect(socket_fd_, reinterpret_cast<sockaddr*>(&sock_address_ipv4_), sizeof(sock_address_ipv4_));
        }

        if (domain_ == DBUS_DOMAIN::DBUS_DOMAIN_UDS) {
            return ::connect(socket_fd_, reinterpret_cast<sockaddr*>(&sock_address_un_), sizeof(sock_address_un_));
        }

        return -1;
    }

    void closeSocketFd() {
        if (socket_fd_ != -1) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
    }

};

} /* opendbus */
