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
#include "socket_client.h"

namespace speed::mq {

socket_client::socket_client (spdmq_ctx_t& ctx) : spdmq_socket(ctx) {
}

socket_client::~socket_client () {
    heart_timer_.expire();
}

int32_t socket_client::connect () {
    if (ctx().domain() == COMM_DOMAIN::IPV4) {
        return ::connect(socket_fd(), reinterpret_cast<sockaddr*>(&sock_address_ipv4()), sizeof(sock_address_ipv4()));
    }

    if (ctx().domain() == COMM_DOMAIN::IPC) {
        return ::connect(socket_fd(), reinterpret_cast<sockaddr*>(&sock_address_un()), sizeof(sock_address_un()));
    }

    return -1;
}

int32_t socket_client::disconnect () {
    return ::close(socket_fd());
}

void socket_client::start_heart (std::function<void()> task) {
    heart_timer_.start(ctx().heartbeat(), task, false);
}

void socket_client::stop_heart () {
    heart_timer_.expire();
}

} /* namespace speed::mq */
