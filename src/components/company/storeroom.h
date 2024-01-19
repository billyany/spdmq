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

#include "spdmq_def.h"
#include "spdmq_queue.hpp"
#include "spdmq_event.h"
#include "spdmq_spinlock.hpp"
#include "spdmq_internal_def.h"

namespace speed::mq {

class storeroom {
private:
    spdmq_queue<comm_msg_t> comm_msg_queue_;
    spdmq_ctx_t& ctx_;
    

public:
    storeroom(spdmq_ctx_t& ctx);
    void comm_msg_queue(comm_msg_t&& msg);
    spdmq_queue<comm_msg_t>& comm_msg_queue();

private:
    spdmq_ctx_t& ctx() {
        return ctx_;
    }
};

} /* namespace speed::mq */