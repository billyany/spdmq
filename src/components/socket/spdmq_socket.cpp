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

#include "spdmq_socket.h"
#include "spdmq_error.hpp"

#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>

namespace speed::mq {

void spdmq_socket::open_socket (int32_t domain, int32_t type, int32_t protocol) {
    socket_fd_ = socket (domain, type | SOCK_CLOEXEC, protocol);
    ERRNO_ASSERT(socket_fd_ >= 3);

    //  Ensure that the socket is closed after the exec call
    const int rc = fcntl (socket_fd_, F_SETFD, FD_CLOEXEC);
    ERRNO_ASSERT (rc != -1);
}

void spdmq_socket::close_socket () {
    if (socket_fd_ >= 3) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

int32_t spdmq_socket::on_read_data(int32_t session_id, uint8_t* buf, int32_t buf_len) {
    int32_t total_bytes_received = 0, bytes_received = 0;

    // 分段读取数据体
    while (total_bytes_received < buf_len) {
        bytes_received = recv(session_id, buf + total_bytes_received, buf_len - total_bytes_received, 0);

        if (bytes_received <= 0) {
            ERRNO_ASSERT (errno != EFAULT && errno != ENOMEM && errno != ENOTSOCK);

            // Interrupted system call
            if (errno == EINTR) {
                continue;
            }

            if (errno == EBADF) { // socket fd close
                return -2;
            }

            return bytes_received;
        }

        total_bytes_received += bytes_received;
    }

    return total_bytes_received;
}

int32_t spdmq_socket::read_data(int32_t session_id, comm_header_t& header, std::vector<uint8_t>& body) {

    // Read data header
    int32_t bytes_received = on_read_data(session_id, (uint8_t*)&header, sizeof header);
    if (bytes_received <= 0) {
        return bytes_received;
    }

    // Read data body
    body.resize(header.comm_msg_len);
    return on_read_data(session_id, body.data(), header.comm_msg_len);
};

int32_t spdmq_socket::write_data(int32_t session_id, const comm_header_t& header, const std::vector<uint8_t>& body) {

    // Send data header
    int32_t ret = send(session_id, &header, sizeof header, MSG_NOSIGNAL);
    if (ret <= 0) {
        return ret;
    }

    // Send data body
    return send(session_id, body.data(), body.size(), MSG_NOSIGNAL);
};

spdmq_ctx_t& spdmq_socket::ctx() {
    return ctx_;
}

fd_t& spdmq_socket::socket_fd () {
    return socket_fd_;
}

sockaddr_un& spdmq_socket::sock_address_un() {
    return sock_address_un_;
}

sockaddr_in& spdmq_socket::sock_address_ipv4() {
    return sock_address_ipv4_;
}

sockaddr_in6& spdmq_socket::sock_address_ipv6() {
    return sock_address_ipv6_;
}

void spdmq_socket::resolve_address() {
    if (ctx().domain() == COMM_DOMAIN::IPV4) {
        sock_address_ipv4_.sin_family = gDomainMap.at(ctx().domain());
        auto url_parse = ctx().config<spdmq_url_parse_t>("url_parse");
        inet_pton(gDomainMap.at(ctx().domain()), url_parse.ip.data(), &(sock_address_ipv4_.sin_addr));
        sock_address_ipv4_.sin_port = htons(url_parse.port);
        return;
    }
    if (ctx().domain() == COMM_DOMAIN::IPV6) {
        return;
    }
    if (ctx().domain() == COMM_DOMAIN::IPC) {
        sock_address_un_.sun_family = gDomainMap.at(ctx().domain());
        auto url_parse = ctx().config<spdmq_url_parse_t>("url_parse");
        strncpy(sock_address_un_.sun_path, url_parse.address.data(), sizeof(sock_address_un_.sun_path) - 1);
        return;
    }
}

spdmq_socket::spdmq_socket(spdmq_ctx_t& ctx): ctx_(ctx) {
}

spdmq_socket::~spdmq_socket () {
    close_socket();
}

} /* namespace speed::mq */
