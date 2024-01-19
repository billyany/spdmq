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

#include "spdmq_mode.h"
#include "spdmq_def.h"
#include <memory>

namespace speed::mq {

spdmq_mode::spdmq_mode(spdmq_ctx& ctx) : ctx_(ctx) {
    dispatcher_ptr_ = std::make_shared<dispatcher>(ctx);
}

spdmq_code_t spdmq_mode::send(spdmq_msg_t& msg) {
    SPDMQ_UNUSED(msg);
    return SPDMQ_CODE_MODE_NOT_MATCH;
}

spdmq_code_t spdmq_mode::recv(spdmq_msg_t& msg, time_msec_t time_out) {
    SPDMQ_UNUSED(msg);
    SPDMQ_UNUSED(time_out);
    return SPDMQ_CODE_MODE_NOT_MATCH;
}

void spdmq_mode::on_recv(comm_msg_t&& msg) {
    if (on_mode_recv) {
        spdmq_msg_t spdmq_msg;
        comm_msg_to_spdmq_msg(msg, spdmq_msg);
        on_mode_recv(spdmq_msg);
    }
}

void spdmq_mode::on_online(comm_msg_t&& msg) {
    // printf("session id:%d\n", msg.session_id);
    if (on_mode_online) {
        spdmq_msg_t spdmq_msg;
        comm_msg_to_spdmq_msg(msg, spdmq_msg);
        on_mode_online(spdmq_msg);
    }
}

void spdmq_mode::on_offline(comm_msg_t&& msg) {
    if (on_mode_offline) {
        spdmq_msg_t spdmq_msg;
        comm_msg_to_spdmq_msg(msg, spdmq_msg);
        on_mode_offline(spdmq_msg);
    }
}

void spdmq_mode::bind() {
    registered();
    dispatcher_ptr_->bind_company(ctx());
}

void spdmq_mode::connect() {
    registered();
    dispatcher_ptr_->connect_company(ctx());
}

void spdmq_mode::spin(bool background) {
    dispatcher_ptr_->operating_company(background);
}

void spdmq_mode::registered() {
    dispatcher_ptr_->registered_company(ctx());

    if (on_mode_recv) {
        dispatcher_ptr_->porter_ptr()->on_recv = [this](auto&& T) {
            on_recv(std::forward<decltype(T)>(T));
        };
    }

    if (on_mode_online) {
        dispatcher_ptr_->porter_ptr()->on_online = [this](auto&& T) {
            on_online(std::forward<decltype(T)>(T));
        };
    }

    if (on_mode_offline) {
        dispatcher_ptr_->porter_ptr()->on_offline = [this](auto&& T) {
            on_offline(std::forward<decltype(T)>(T));
        };
    }
}

} /* namespace speed::mq */
