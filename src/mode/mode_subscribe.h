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

#include "spdmq_mode.h"

namespace speed::mq {

class mode_subscribe : public spdmq_mode {
public:
    mode_subscribe(spdmq_ctx& ctx, spdmq_callback_t& on_recv, spdmq_callback_t& on_online, spdmq_callback_t& on_offline);
    void registered() override;
    void on_online(comm_msg_t&& msg) override;
    spdmq_code_t recv(spdmq_msg_t& msg, time_msec_t time_out) override;
};

} /* speed::mq */