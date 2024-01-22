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

#include "mode_subscribe.h"

namespace speed::mq {

mode_subscribe::mode_subscribe(spdmq_ctx& ctx, spdmq_callback_t& on_recv, spdmq_callback_t& on_online, spdmq_callback_t& on_offline) 
    : spdmq_mode(ctx, on_recv, on_online, on_offline) {
}

void mode_subscribe::registered() {

    handler()->registered_company(ctx());

    if (on_mode_recv) {
        handler()->porter_ptr()->on_recv = [this](auto&& T) {
            on_recv(std::forward<decltype(T)>(T));
        };
    }

    handler()->porter_ptr()->on_online = [this](auto&& T) {
        on_online(std::forward<decltype(T)>(T));
    };

    if (on_mode_offline) {
        handler()->porter_ptr()->on_offline = [this](auto&& T) {
            on_offline(std::forward<decltype(T)>(T));
        };
    }
}

void mode_subscribe::on_online(comm_msg_t&& msg) {
    for (auto& topic : ctx().topics()) {
        msg.topic = topic;
        msg.msg_type = MESSAGE_TYPE::TOPIC;
        handler()->porter_ptr()->send_msg(msg.session_id, msg);

    }

    if (on_mode_online) {
        spdmq_msg_t spdmq_msg;
        comm_msg_to_spdmq_msg(msg, spdmq_msg);
        on_mode_online(spdmq_msg);
    }
}

spdmq_code_t mode_subscribe::recv(spdmq_msg_t& msg, time_msec_t time_out) {
    comm_msg_t comm_msg;
    auto ret = handler()->porter_ptr()->recv_msg(handler()->spdmq_socket_ptr()->socket_fd(), comm_msg, time_out);
    if (ret == SPDMQ_CODE_OK) {
        comm_msg_to_spdmq_msg(comm_msg, msg);
    }
    return ret;
}

} /* namespace speed::mq */