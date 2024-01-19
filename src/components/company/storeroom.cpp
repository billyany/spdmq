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

#include "storeroom.h"

namespace speed::mq {

storeroom::storeroom(spdmq_ctx_t& ctx) : ctx_(ctx) {}

void storeroom::comm_msg_queue(comm_msg_t&& msg) {
    if (comm_msg_queue_.size() < ctx().queue_size()) {
        comm_msg_queue_.push(std::forward<comm_msg_t>(msg));
        return;
    }
    comm_msg_queue_.pop();
    comm_msg_queue_.push(std::forward<comm_msg_t>(msg));
}

spdmq_queue<comm_msg_t>& storeroom::comm_msg_queue() {
    return comm_msg_queue_;
}

} /* namespace speed::mq */

