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

#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "base_socket.h"
#include "inet_address.h"
#include "spdmq_func.hpp"
#include "spdmq_logger.hpp"

namespace speed::mq {

base_socket::base_socket(int sockfd) : sockfd_(sockfd) {}

base_socket::~base_socket() { close(sockfd_); }

int base_socket::fd() const { return sockfd_; }

bool base_socket::get_tcp_info(struct tcp_info* tcpi) const {
    socklen_t len = sizeof(*tcpi);
    mem_zero(tcpi, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool base_socket::get_tcp_info_string(char* buf, int len) const {
    struct tcp_info tcpi;
    bool ok = get_tcp_info(&tcpi);
    if (ok) {
        snprintf(buf, len, "unrecovered=%u "
            "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
            "lost=%u retrans=%u rtt=%u rttvar=%u "
            "sshthresh=%u cwnd=%u total_retrans=%u",
            tcpi.tcpi_retransmits,   // 未恢复的[RTO]超时次数
            tcpi.tcpi_rto,           // 重传超时时间（微秒）
            tcpi.tcpi_ato,           // 预测的软时钟滴答时间（微秒）
            tcpi.tcpi_snd_mss,       // 发送方的最大段大小
            tcpi.tcpi_rcv_mss,       // 接收方的最大段大小
            tcpi.tcpi_lost,          // 丢失的数据包数量
            tcpi.tcpi_retrans,       // 重传的数据包数量
            tcpi.tcpi_rtt,           // 平滑的往返时间（微秒）
            tcpi.tcpi_rttvar,        // 中等偏差
            tcpi.tcpi_snd_ssthresh,  // 拥塞窗口慢启动门限
            tcpi.tcpi_snd_cwnd,      // 拥塞窗口大小
            tcpi.tcpi_total_retrans);// 整个连接的总重传次数
    }
    return ok;
}

void base_socket::bind_address(const inet_address& addr) {
    int ret = ::bind(sockfd_, addr.get_sock_addr(), static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        LOG_FATAL("bind addr:%s failed", addr.to_ip_port().data());
    }
}

void base_socket::listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        LOG_FATAL("listen sockfd:%d failed", sockfd_);
    }
}

int base_socket::accept(inet_address* peeraddr) {
    struct sockaddr_in6 addr;
    mem_zero(&addr, sizeof addr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof addr);
    int connfd = ::accept4(sockfd_, reinterpret_cast<sockaddr*>(&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd < 0) {
        int saved_errno = errno;
        switch (saved_errno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL("unexpected error of ::accept errno:%s", std::strerror(saved_errno));
                break;
            default:
                LOG_FATAL("unknown error of ::accept errno:%s", std::strerror(saved_errno));
                abort();
                break;
        }
    }

    if (connfd >= 0) {
        peeraddr->set_sock_addr_inet6(addr);
    }
    return connfd;
}

void base_socket::shutdown_write() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("shutdown_write");
    }
}

void base_socket::set_tcp_no_delay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

void base_socket::set_reuse_addr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void base_socket::set_reuse_port(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on) {
        LOG_ERROR("SO_REUSEPORT failed.");
    }
#else
    if (on) {
        LOG_ERROR("SO_REUSEPORT is not supported.");
    }
#endif
}

void base_socket::set_keep_alive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

} /* namespace speed::mq */
