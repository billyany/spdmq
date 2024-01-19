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

#include "mode_publish.h"
#include "spdmq_spinlock.hpp"

namespace speed::mq {

mode_publish::mode_publish(spdmq_ctx& ctx) : spdmq_mode(ctx) {
}

spdmq_code_t mode_publish::send(spdmq_msg_t& msg) {
    comm_msg_t comm_msg;
    spdmq_msg_to_comm_msg(msg, comm_msg);
    comm_msg.msg_type = MESSAGE_TYPE::DATA;
    spdmq_spinlock<std::atomic_flag> lk(lock_);
    // printf("mode_publish::send before\n");
    for (auto& session_id : subscribe_table_[comm_msg.topic]) {
        // printf("mode_publish::send in\n");
        handler()->porter_ptr()->send_msg(session_id, comm_msg);
    }

    return SPDMQ_CODE_OK;
}

void mode_publish::registered() {

    handler()->registered_company(ctx());

    handler()->porter_ptr()->on_recv = [this](auto&& T) {
        on_recv(std::forward<decltype(T)>(T));
    };

    if (on_mode_online) {
        handler()->porter_ptr()->on_online = [this](auto&& T) {
            on_online(std::forward<decltype(T)>(T));
        };
    }

    handler()->porter_ptr()->on_offline = [this](auto&& T) {
        on_offline(std::forward<decltype(T)>(T));
    };
}

void mode_publish::on_recv(comm_msg_t&& msg) {
    msg_deal(msg);
}

void mode_publish::on_offline(comm_msg_t&& msg) {
    session_remove(msg.session_id);
    if (on_mode_offline) {
        spdmq_msg_t spdmq_msg;
        comm_msg_to_spdmq_msg(msg, spdmq_msg);
        on_mode_offline(spdmq_msg);
    }
}

void mode_publish::msg_deal(const comm_msg_t& msg) {
    // printf("msg_deal session id:%d\n", msg.session_id);
    if (MESSAGE_TYPE::TOPIC == msg.msg_type) {
        // printf("msg_deal in\n");
        topic_insert(msg.session_id, msg.topic);
    }
}

void mode_publish::topic_insert(fd_t session_id, const std::string& topic) {
    spdmq_spinlock<std::atomic_flag> lk(lock_);
    // printf("topic_insert session id:%d, topic:%s\n", session_id, topic.data());
    subscribe_table_[topic].insert(session_id);
}

void mode_publish::session_remove(fd_t session_id) {
    spdmq_spinlock<std::atomic_flag> lk(lock_);
    for (auto& topic_set : subscribe_table_) {
        topic_set.second.erase(session_id);
    }
}

} /* speed::mq */