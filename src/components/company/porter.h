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

#include "storeroom.h"
#include "spdmq_event.h"
#include "spdmq_socket.h"
#include "spdmq_internal_def.h"

namespace speed::mq {

class porter
{
private:
    spdmq_ctx_t& ctx_;
    std::shared_ptr<spdmq_event> spdmq_event_ptr_;
    std::shared_ptr<spdmq_socket> spdmq_socket_ptr_;
    std::shared_ptr<storeroom> storeroom_ptr_;
    std::mutex lock_;
    std::condition_variable cv_;

public:
    std::function<void(comm_msg_t&&)> on_recv;
    std::function<void(comm_msg_t&&)> on_online;
    std::function<void(comm_msg_t&&)> on_offline;

public:
    porter(spdmq_ctx_t& ctx,
           std::shared_ptr<spdmq_event> spdmq_event_ptr, 
           std::shared_ptr<spdmq_socket> spdmq_socket_ptr,
           std::shared_ptr<storeroom> storeroom_ptr);

    int32_t send_msg(int32_t session_id, const comm_msg_t& comm_msg);
    int32_t recv_msg(int32_t session_id, comm_msg_t& comm_msg, time_msec_t time_out);

    void on_reconnect();
    void on_read(int32_t session_id);
    void on_connecting(int32_t session_id);
    void on_connected(int32_t session_id);
    void on_disconnect(int32_t session_id);

private:
    int32_t on_send_msg(int32_t session_id, const comm_msg& msg);
    spdmq_queue<comm_msg_t>& queue();
    spdmq_ctx_t& ctx();
};

} // namespace speed::mq
