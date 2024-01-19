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

#include "porter.h"
#include <cstdio>

namespace speed::mq {

porter::porter (spdmq_ctx_t& ctx,
                std::shared_ptr<spdmq_event> spdmq_event_ptr, 
                std::shared_ptr<spdmq_socket> spdmq_socket_ptr,
                std::shared_ptr<storeroom> storeroom_ptr)
    : ctx_(ctx),
      spdmq_event_ptr_ (spdmq_event_ptr), 
      spdmq_socket_ptr_ (spdmq_socket_ptr),
      storeroom_ptr_ (storeroom_ptr)
{
        std::thread([this] {
            while (true) {
                std::unique_lock<std::mutex> lk(lock_);
                cv_.wait(lk, [&] { return !queue().empty() && on_recv; });
                
                comm_msg_t comm_msg;
                queue().pop(comm_msg);
                on_recv(std::move(comm_msg));
            }
        }).detach();
}

int32_t porter::send_msg(int32_t session_id, const comm_msg_t& comm_msg) {
    // printf("comm_msg.payload size:%lu\n", comm_msg.payload.size());
    return on_send_msg(session_id, comm_msg);
}

int32_t porter::recv_msg(int32_t session_id, comm_msg_t& comm_msg, time_msec_t time_out) {
    
    // The received callback has intercepted the data
    if (on_recv) {
        return SPDMQ_CODE_RECV_CB_INTERCEPTED_DATA;
    }

    if (queue().empty()) {
        // non-blocking mode
        if (time_out < 0) {
            return SPDMQ_CODE_NO_DATA;
        }

        // blocking mode
        if (time_out == 0) {
            std::unique_lock<std::mutex> lk(lock_);
            cv_.wait(lk, [&] { return !queue().empty(); });
        }
        else if (time_out > 0) {
            std::unique_lock<std::mutex> lk(lock_);
            using namespace std::chrono_literals;
            if (!cv_.wait_for(lk, time_out * 1ms,[&] { return !queue().empty(); })) {
                return SPDMQ_CODE_NO_DATA;
            }
        }
    }

    queue().pop(comm_msg);
    // printf("pop comm_msg.payload size :%lu\n", comm_msg.payload.size());

    return SPDMQ_CODE_OK;
}

void porter::on_reconnect() {
    std::thread([this] {
        while (true) {
            if (!spdmq_socket_ptr_->connect()) {
                // Add socket fd to event loop
                spdmq_event_ptr_->event_add(spdmq_socket_ptr_->socket_fd());
                // printf("connect success, spdmq_socket_ptr_->socket_fd:%d\n", spdmq_socket_ptr_->socket_fd());

                // Add connection events to the event loop
                // printf("spdmq_event_ptr_->urgent_event\n");
                spdmq_event_ptr_->urgent_event({spdmq_socket_ptr_->socket_fd(), EVENT::CONNECTED});
                break;
            }
            // printf("reconnect_interval:%d\n", ctx().reconnect_interval());
            if (ctx().reconnect_interval() == 0) {
                break;
            }

            sleep_ms(ctx().reconnect_interval());
        }
    }).detach();
}

void porter::on_read(int32_t session_id) {
    // printf("porter::on_read\n");
    while (true) {
        comm_header header;
        std::vector<uint8_t> body;
        auto rc = spdmq_socket_ptr_->read_data(session_id, header, body);
        // printf("rc:%d\n", rc);
        if (rc <= 0) {
            // printf("rc:%d, error msg:%s\n", rc, std::strerror(errno));
            // spdmq_event_ptr_->event_del(spdmq_socket_ptr_->socket_fd());
            // spdmq_event_ptr_->urgent_event({spdmq_socket_ptr_->socket_fd(), EVENT::DISCONNECT});
            return;
        }
        // printf("rc:%d\n", rc);

        // Deserialize comm_msg_t
        comm_msg_t comm_msg;
        deserialize_comm_msg_t(body, comm_msg);
        comm_msg.session_id = session_id;
        // printf("comm_msg.payload size :%lu\n", comm_msg.payload.size());

        // Update heartbeat status
        if (MESSAGE_TYPE::HEARTBEAT == comm_msg.msg_type) {
            spdmq_event_ptr_->update_session(session_id);
            return;
        }

        storeroom_ptr_->comm_msg_queue(std::move(comm_msg));
        cv_.notify_all();
    }
}

void porter::on_connecting(int32_t session_id) {
    while (true) {
        auto client_fd = spdmq_socket_ptr_->accept(session_id);
        if (client_fd > 3) {
            spdmq_event_ptr_->urgent_event({client_fd, EVENT::CONNECTED});
        }
        else {
            // printf("client_fd failed failed\n");
            break;
        }
    }

}

void porter::on_connected(int32_t session_id) {
    // printf("porter::on_connected session_id:%d\n", session_id);
    if (session_id != spdmq_socket_ptr_->socket_fd()) {
        spdmq_event_ptr_->event_add(session_id);
        spdmq_event_ptr_->update_session(session_id);
    }
    else {
        spdmq_socket_ptr_->start_heart([this] {
            comm_msg msg;
            msg.session_id = spdmq_socket_ptr_->socket_fd();
            msg.msg_type = MESSAGE_TYPE::HEARTBEAT;
            // printf("send heartbeat\n");
            auto ret = send_msg(spdmq_socket_ptr_->socket_fd(), msg);
            // printf("ret:%d\n", ret);
            if (ret != 0) {
                // printf("ret:%d\n", ret);
                spdmq_event_ptr_->event_del(spdmq_socket_ptr_->socket_fd());
                spdmq_event_ptr_->urgent_event({spdmq_socket_ptr_->socket_fd(), EVENT::DISCONNECT});
            }
        });
    }
    if (on_online) {
        on_online(session_id);
    }
}

void porter::on_disconnect(int32_t session_id) {
    if (session_id != spdmq_socket_ptr_->socket_fd()) {
        spdmq_event_ptr_->event_del(session_id);
        spdmq_event_ptr_->remove_session(session_id);
    }
    else {
        // printf("porter::on_disconnect session_id:%d\n", session_id);
        spdmq_socket_ptr_->stop_heart();
        close(spdmq_socket_ptr_->socket_fd());
        if (ctx().reconnect_interval()) {
            spdmq_socket_ptr_->open_socket();
            on_reconnect();
        }
    }
    if (on_offline) {
        on_offline(session_id);
    }
}

int32_t porter::on_send_msg(int32_t session_id, const comm_msg& msg) {
    comm_header header;
    header.comm_msg_len = msg.size();
    std::vector<uint8_t> body;
    serialize_comm_msg_t(msg, body);

    comm_msg msg1;
    deserialize_comm_msg_t(body, msg1);
    // printf("msg1.payload size:%lu, body size:%lu\n", msg1.payload.size(), body.size());

    auto ret = spdmq_socket_ptr_->write_data(session_id, header, body);
    if (ret < 0) {
        // printf("session_id:%d, ret:%d, errno:%s\n", session_id, ret, std::strerror(errno));
        // TODO "Are you considering doing something with the session ID ?"
        // spdmq_event_ptr_->event_del(session_id);
        // spdmq_event_ptr_->urgent_event({session_id, EVENT::DISCONNECT});
        return SPDMQ_CODE_CONNECT_TO_BROKEN;
    }
    return SPDMQ_CODE_OK;
}

spdmq_queue<comm_msg_t>& porter::queue() {
    return storeroom_ptr_->comm_msg_queue();
}

spdmq_ctx_t& porter::ctx() {
    return ctx_;
}

} /* namespace speed::mq */
