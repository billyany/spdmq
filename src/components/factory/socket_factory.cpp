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

#include "tcp_server.h"
#include "tcp_client.h"
#include "uds_server.h"
#include "uds_client.h"
#include "socket_factory.h"
#include "spdmq_internal_def.h"

namespace speed::mq {

server_factory* server_factory::instance() {
    static server_factory impl;
    return &impl;
}

std::shared_ptr<spdmq_socket> server_factory::create_socket(spdmq_ctx_t& ctx) {
    std::shared_ptr<socket_server> socket_ptr;
    auto socket_mode = ctx.config<socket_mode_t>("socket_mode");
    switch (socket_mode) {
        case SOCKET_MODE::TCP:
            socket_ptr = std::make_shared<tcp_server>(ctx);
            break;
        case SOCKET_MODE::UDP:
            break;
        case SOCKET_MODE::UDS:
            socket_ptr = std::make_shared<uds_server>(ctx);
            break;
    }
    return socket_ptr;
}

client_factory* client_factory::instance() {
    static client_factory impl;
    return &impl;
}

std::shared_ptr<spdmq_socket> client_factory::create_socket(spdmq_ctx_t& ctx) {
    std::shared_ptr<socket_client> socket_ptr;
    auto socket_mode = ctx.config<socket_mode_t>("socket_mode");
    switch (socket_mode) {
        case SOCKET_MODE::TCP:
            socket_ptr = std::make_shared<tcp_client>(ctx);
            break;
        case SOCKET_MODE::UDP:
            break;
        case SOCKET_MODE::UDS:
            socket_ptr = std::make_shared<uds_client>(ctx);
            break;
    }
    return socket_ptr;
}

} /* namespace speed::mq */