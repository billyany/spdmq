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

#include "tcp_client.h"
#include "spdmq_socket.h"

namespace speed::mq {

tcp_client::tcp_client(spdmq_ctx_t& ctx) : socket_client(ctx) {}

void tcp_client::open_socket () {
    spdmq_socket::open_socket(gDomainMap.at(ctx().domain()), SOCK_STREAM, 0);
}

} /* namespace speed::mq */