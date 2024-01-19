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

#include <cstdio>
#include <memory>
#include <thread>
#include "spdmq_event.h"

namespace speed::mq {

spdmq_event::spdmq_event(spdmq_ctx_t& ctx) : ctx_(ctx) {
    session_map_list_.resize(4);
}

spdmq_event::~spdmq_event() {}

spdmq_ctx_t& spdmq_event::ctx() {
    return ctx_;
}

void spdmq_event::event_run(bool background) {
    // session_timer_.start(ctx().heartbeat(), std::bind(&spdmq_event::session_clear, this));
    std::thread event_thread(std::bind(&spdmq_event::event_loop, this));
    if (background) {
        event_thread.detach();
    }
    else {
        event_thread.join();
    }
}

void spdmq_event::event_stop() {
    stop_event_loop_.store(true);
    notify_event();
    session_timer_.expire();
}

void spdmq_event::normal_event(std::pair<int32_t, EVENT> event) {
    normal_queue_.push(event);
    notify_event();
}

void spdmq_event::urgent_event(std::pair<int32_t, EVENT> event) {
    urgent_queue_.push(event);
    notify_event();
}

void spdmq_event::notify_event() {
    // printf("cv_.notify_all before\n");
    cv_.notify_one();
    // printf("cv_.notify_all after\n");
}

void spdmq_event::update_session(fd_t session_id) {
    spdmq_spinlock<std::atomic_flag> lk(atomic_lock_);
    for (auto& session_map : session_map_list_) {
        auto it = session_map.find(session_id);
        if (it != session_map.end()) {
            session_map_list_[heartbeat_loop_cnt_ % 4][session_id] = it->second;
            return;
        }
    }
    session_map_list_[heartbeat_loop_cnt_ % 4][session_id] = std::make_shared<spdmq_session>(session_id);
}

void spdmq_event::remove_session(fd_t session_id) {
    spdmq_spinlock<std::atomic_flag> lk(atomic_lock_);
    for (auto& session_map : session_map_list_) {
        auto it = session_map.find(session_id);
        if (it != session_map.end()) {
            session_map.erase(it);
        }
    }
}

void spdmq_event::session_clear() {
    spdmq_spinlock<std::atomic_flag> lk(atomic_lock_);
    session_map_list_[heartbeat_loop_cnt_++ % 4].clear();
}

void spdmq_event::event_loop() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(lock_);
            cv_.wait(lk, [this] {
                // printf("wait\n");
                return !normal_event().empty() || !urgent_event().empty() || stop_event_loop_; 
            });
        }
        // printf("wait out2\n");
        if (stop_event_loop_) break;
        // printf("wait out2\n");

        event_consume(urgent_event());
        event_consume(normal_event());
        // printf("wait out3\n");
    }
}

template<typename T>
void spdmq_event::event_consume(T& queue) {
    while (!queue.empty()) {
        auto event = queue.pop();
        if (EVENT::READ == event.second && on_read) {
            // printf("EVENT::READ\n");
            on_read(event.first);
            continue;
        }
        if (EVENT::CONNECTING == event.second && on_connecting) {
            // printf("EVENT::CONNECTING\n");
            on_connecting(event.first);
            continue;
        }
        if (EVENT::CONNECTED == event.second && on_connected) {
            // printf("EVENT::CONNECTED\n");
            on_connected(event.first);
            continue;
        }
        if (EVENT::DISCONNECT == event.second && on_disconnect) {
            // printf("EVENT::DISCONNECT\n");
            on_disconnect(event.first);
            continue;
        }
    }
}

set_queue<std::pair<int32_t, EVENT>>& spdmq_event::normal_event() {
    return normal_queue_;
}

spdmq_queue<std::pair<int32_t, EVENT>>& spdmq_event::urgent_event() {
    return urgent_queue_;
}

} /* namespace speed::mq */
