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
    std::thread event_thread(std::bind(&spdmq_event::event_loop, this));

    if (ctx().has_config("server_fd")) {
        session_clear_timer_.start(ctx().heartbeat(), [this] () {
            session_clear();
        });

        heartbeat_cnt_timer_.start(ctx().heartbeat(), [this] () {
            heartbeat_loop_cnt_.fetch_add(1);
            heartbeat_clear_cnt_.fetch_add(1);
        });
    }

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
    session_clear_timer_.expire();
    heartbeat_cnt_timer_.expire();
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
    cv_.notify_one();
}

void spdmq_event::update_session(fd_t session_id) {
    spdmq_spinlock<std::atomic_flag> lk(atomic_lock_);
    for (auto& session_map : session_map_list_) {
        auto it = session_map.find(session_id);
        if (it != session_map.end()) {
            session_map_list_[heartbeat_loop_cnt_.load() % HEARTBEAT_RESETTING][session_id] = it->second;
            return;
        }
    }
    auto session_id_ptr = std::make_shared<spdmq_session>(session_id);
    // session_id_ptr->on_notify_event = [this] (fd_t session_id) {
    //     urgent_event().push({session_id, EVENT::DISCONNECT});
    //     notify_event();
    // };
    session_map_list_[heartbeat_loop_cnt_.load() % HEARTBEAT_RESETTING][session_id] = session_id_ptr;

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
    session_map_list_[heartbeat_clear_cnt_.load() % HEARTBEAT_RESETTING].clear();
}

void spdmq_event::event_loop() {
    while (true) {
        {
            std::unique_lock<std::mutex> lk(lock_);
            cv_.wait(lk, [this] {
                return !normal_event().empty() || !urgent_event().empty() || stop_event_loop_; 
            });
        }
        if (stop_event_loop_) break;

        event_consume(urgent_event());
        event_consume(normal_event());
    }
}

template<typename T>
void spdmq_event::event_consume(T& queue) {
    while (!queue.empty()) {
        auto event = queue.pop();
        if (EVENT::READ == event.second && on_read) {
            on_read(event.first);
            continue;
        }
        if (EVENT::CONNECTING == event.second && on_connecting) {
            on_connecting(event.first);
            continue;
        }
        if (EVENT::CONNECTED == event.second && on_connected) {
            on_connected(event.first);
            continue;
        }
        if (EVENT::DISCONNECT == event.second && on_disconnect) {
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
