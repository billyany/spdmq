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

#include "uds_server.h"

namespace speed::mq {

uds_server::uds_server(spdmq_ctx_t& ctx) : socket_server(ctx) {}

void uds_server::open_socket () {
    spdmq_socket::open_socket(gDomainMap.at(ctx().domain()), gProtocolTypeMap.at(ctx().protocol_type()), 0);
}

} /* namespace speed::mq */