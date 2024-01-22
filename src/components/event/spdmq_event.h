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

#include <memory>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "spdmq_def.h"
#include "spdmq_session.h"
#include "spdmq_timer.hpp"
#include "spdmq_queue.hpp"
#include "event_struct.h"
#include "spdmq_internal_def.h"

#define HEARTBEAT_RESETTING 4

namespace speed::mq {

class spdmq_event {
public:
    std::function<void(int32_t)> on_read;       // Read event callback
    std::function<void(int32_t)> on_connecting; // Callback for in progress connection events
    std::function<void(int32_t)> on_connected;  // Callback for completed connection events
    std::function<void(int32_t)> on_disconnect; // Disconnect event callback

private:
    spdmq_ctx_t& ctx_;
    std::mutex lock_;
    std::condition_variable cv_;
    std::atomic_bool stop_event_loop_ = false;
    set_queue<std::pair<int32_t, EVENT>> normal_queue_;
    spdmq_queue<std::pair<int32_t, EVENT>> urgent_queue_;

    spdmq_timer session_clear_timer_, heartbeat_cnt_timer_;
    std::atomic_uint64_t heartbeat_loop_cnt_ = 2;
    std::atomic_uint64_t heartbeat_clear_cnt_ = 0;
    std::atomic_flag atomic_lock_ = ATOMIC_FLAG_INIT;
    std::vector<std::map<fd_t, std::shared_ptr<spdmq_session>>> session_map_list_;

public:
    virtual void event_add(fd_t fd) = 0;
    virtual void event_del(fd_t fd) = 0;
    virtual void event_create() = 0;
    virtual void event_build() = 0;
    virtual void event_destroy() = 0;

public:
    spdmq_event(spdmq_ctx_t& ctx);
    virtual ~spdmq_event();
    spdmq_ctx_t& ctx();

    void event_run(bool background);
    void event_stop();
    void normal_event(std::pair<int32_t, EVENT> event);
    void urgent_event(std::pair<int32_t, EVENT> event);
    void update_session(fd_t session_id);
    void remove_session(fd_t session_id);

protected:
    void notify_event();

private:
    void session_clear();
    void event_loop();
    template<typename T>
    void event_consume(T& queue);
    set_queue<std::pair<int32_t, EVENT>>& normal_event();
    spdmq_queue<std::pair<int32_t, EVENT>>& urgent_event();
};

} /* namespace speed::mq */
