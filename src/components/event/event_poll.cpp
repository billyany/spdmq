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

#include "event_poll.h"
#include <thread>
#include <cstdint>
#include <future>

namespace speed::mq {

event_poll::event_poll(spdmq_ctx_t& ctx) : spdmq_event(ctx) {

}

void event_poll::event_add(fd_t fd) {
    epoll_event evt;
    evt.events = gEventModeMap.at(ctx().event_mode());
    evt.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &evt);
}

void event_poll::event_del(fd_t fd) {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
}

void event_poll::event_create() {
    epoll_fd_ = epoll_create1(0);
    ERRNO_ASSERT(epoll_fd_ != -1);
}

void event_poll::event_build() {
    // std::async(std::launch::async, &event_poll::event_poll_loop, this);
    std::thread(&event_poll::event_poll_loop, this).detach();
}

void event_poll::event_destroy() {
    destroy_event_loop_.store(true);
}

void event_poll::event_poll_loop() {
    // printf("event_poll_loop\n");
    auto evt_num = ctx().evt_num() < 100 ? 10 : ctx().evt_num() / 10;
    std::shared_ptr<epoll_event> events_ptr(new epoll_event[evt_num](), [] (epoll_event* events) { delete [] events; });
    auto server_fd = -1;
    if (ctx().has_config("server_fd")) {
        // printf("server_fd:%d\n", server_fd);
        server_fd = ctx().config<fd_t>("server_fd");

        // Prevent blocking caused by server binding and listening before this thread runs, 
        // and client connecting without any new connections in the future
        urgent_event({server_fd, EVENT::CONNECTING});
    }
    

    while (true) {
        
        epoll_event* events = events_ptr.get();
        auto curr_events = epoll_wait(epoll_fd_, events, evt_num, 1000);
        ERRNO_ASSERT(curr_events != -1);
        
        if (destroy_event_loop_.load()) break;

        // traverse events
        for (auto i = 0; i < curr_events; ++i) {
            // printf("events[i].data.fd:%d\n", events[i].data.fd);
            if (-1 == events[i].data.fd) continue;
            if (events[i].data.fd == server_fd && ctx().protocol_type() == COMM_PROTOCOL_TYPE::TCP) {
                // printf("EVENT::CONNECTING events[i].data.fd:%d\n", events[i].data.fd);
                urgent_event({static_cast<int32_t>(events[i].data.fd), EVENT::CONNECTING});
            }
            else {
                normal_event({static_cast<int32_t>(events[i].data.fd), EVENT::READ});
            }
        }
    }
}

} /* namespace speed::mq */