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

#include <cstdio>
#include <unistd.h>
#include "socket_server.h"
#include "spdmq_error.hpp"
#include "spdmq_internal_def.h"

namespace speed::mq {

socket_server::socket_server (spdmq_ctx_t& ctx) : spdmq_socket(ctx) {
}

void socket_server::bind () {
    if (ctx().domain() == COMM_DOMAIN::IPV4) {
        // enable address reuse
        int32_t optval = 1; // 1 - enable, 0 - disenable
        int32_t rc = setsockopt(socket_fd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        SOCKET_ASSERT (rc != -1, socket_fd());

        // bind socket_fd to address
        rc = ::bind(socket_fd(), reinterpret_cast<sockaddr*>(&sock_address_ipv4()), sizeof(sock_address_ipv4()));
        SOCKET_ASSERT (rc != -1, socket_fd());
    }

    if (ctx().domain() == COMM_DOMAIN::IPC) {

        auto url_parse = ctx().config<spdmq_url_parse_t>("url_parse");
        std::string lock_address = url_parse.address + ".lock";
        
        // if the lock is unsuccessful, throw an exception
        file_lock_ = std::make_shared<spdmq_filelock>(lock_address, true);
        if (access(url_parse.address.c_str(), F_OK) == 0) {
            unlink(url_parse.address.c_str());
        }
        
        // bind socket_fd to address
        int32_t rc = ::bind(socket_fd(), reinterpret_cast<sockaddr*>(&sock_address_un()), sizeof(sock_address_un()));
        SOCKET_ASSERT (rc != -1, socket_fd());
    }
}

void socket_server::listen () {
    int32_t rc = ::listen(socket_fd(), ctx().evt_num());
    SOCKET_ASSERT (rc != -1, socket_fd());
}

fd_t socket_server::accept (fd_t server_fd) {

    struct timeval timeout = {0,100 * 1000};
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

    fd_t client_fd = -1;
    if (ctx().domain() == COMM_DOMAIN::IPV4) {
        sockaddr_in client_address;
        socklen_t client_address_size = sizeof(client_address);
        client_fd = ::accept(server_fd, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);
    }

    if (ctx().domain() == COMM_DOMAIN::IPC) {
        client_fd = ::accept(server_fd, nullptr, nullptr);
    }
    
    return client_fd;
}

} /* namespace speed::mq */