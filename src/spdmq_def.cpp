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

#include "spdmq_def.h"

namespace speed::mq {

/* variable set */
spdmq_ctx& spdmq_ctx::mode(comm_mode_t mode) {
    _mode = mode;
    return *this;
}

spdmq_ctx& spdmq_ctx::method(comm_method_t method) {
    _method = method;
    return *this;
}

spdmq_ctx& spdmq_ctx::domain(comm_domain_t domain)
{
    _domain = domain;
    return *this;
}

spdmq_ctx& spdmq_ctx::protocol(comm_protocol_t protocol)
{
    _protocol = protocol;
    return *this;
}

spdmq_ctx& spdmq_ctx::event_mode(event_mode_t event_mode)
{
    _event_mode = event_mode;
    return *this;
}

spdmq_ctx& spdmq_ctx::evt_num(int32_t evt_num) {
    _evt_num = evt_num;
    return *this;
}

spdmq_ctx& spdmq_ctx::heartbeat(int32_t heartbeat) {
    _heartbeat = heartbeat;
    return *this;
}

spdmq_ctx& spdmq_ctx::queue_size(int32_t queue_size) {
    _queue_size = queue_size;
    return *this;
}

spdmq_ctx& spdmq_ctx::topics(std::set<std::string> topics) {
    _topics = topics;
    return *this;
}

/* variable get */
comm_mode_t spdmq_ctx::mode() {
    return _mode;
}

comm_method_t spdmq_ctx::method() {
    return _method;
}

comm_domain_t spdmq_ctx::domain()
{
    return _domain;
}

comm_protocol_t spdmq_ctx::protocol()
{
    return _protocol;
}

event_mode_t spdmq_ctx::event_mode() 
{
    return _event_mode;
}

int32_t spdmq_ctx::evt_num() {
    return _evt_num;
}

int32_t spdmq_ctx::heartbeat() {
    return _heartbeat;
}

int32_t spdmq_ctx::queue_size() {
    return _queue_size;
}

std::set<std::string> spdmq_ctx::topics() {
    return _topics;
}

} /* speed::mq */