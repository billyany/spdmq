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

#pragma once

#include <map>
#include <string>
#include <cstdint>
#include <sys/socket.h>

#include "spdmq_def.h"

namespace speed::mq {

const std::map<comm_domain_t, int32_t> gDomainMap = {
    {COMM_DOMAIN::IPV4, AF_INET},
    {COMM_DOMAIN::IPV6, AF_INET6},
    {COMM_DOMAIN::IPC, AF_UNIX},
};

const std::map<comm_protocol_type_t, int32_t> gProtocolTypeMap = {
    {COMM_PROTOCOL_TYPE::TCP, SOCK_STREAM},
    {COMM_PROTOCOL_TYPE::UDP, SOCK_DGRAM},
};

} /* namespace speed::mq */