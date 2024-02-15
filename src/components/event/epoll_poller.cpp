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

#include "spdmq_func.hpp"
#include "epoll_poller.h"
#include "socket_channel.h"

#include <assert.h>
#include <cstdlib>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

namespace speed::mq {

epoll_poller::epoll_poller(event_loop& loop)
    : base_poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        // TODO: 打印点什么或者抛出异常时带出原因和位置
        abort();
    }
}

epoll_poller::~epoll_poller() {
  ::close(epollfd_);
}

void epoll_poller::poll(int32_t timeout_ms, channel_list_t* activec_channels) {
    int32_t num_events = ::epoll_wait(epollfd_, 
                                 events_.data(), 
                                 static_cast<int32_t>(events_.size()), 
                                 timeout_ms);

    int saved_errno = errno;
    if (num_events > 0) {
        fill_active_channels(num_events, activec_channels);
        if (static_cast<int32_t>(num_events) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
        return;
    }
    if (num_events == 0) {
        // TODO: 打印点日志
        // LOG_TRACE << "nothing happened";
        return;
    }
    if (num_events < 0) {
        // error happens, log uncommon ones
        if (saved_errno != EINTR) {
            errno = saved_errno;
            // TODO: 打印点日志
            // LOG_SYSERR << "epoll_poller::poll()";
        }
        return;
    }
}

void epoll_poller::fill_active_channels(int32_t num_events, channel_list_t* activec_channels) const {
    assert(static_cast<std::size_t>(num_events) <= events_.size());
    for (int32_t i = 0; i < num_events; ++i) {
        auto channel = static_cast<socket_channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        auto it = channels_.find(channel->fd());
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activec_channels->emplace_back(channel);
    }
}

void epoll_poller::update_channel(socket_channel* channel) {
    assert_in_loop_thread();
    const int index = channel->index();
    //   LOG_TRACE << "fd = " << channel->fd()
    //     << " events = " << channel->events() << " index = " << index;
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else { // index == kDeleted
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else {
        // update existing one with EPOLL_CTL_MOD/DEL
#ifndef NDEBUG
        int fd = channel->fd();
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
#endif
        if (channel->is_none_event()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void epoll_poller::remove_channel(socket_channel* channel) {
    assert_in_loop_thread();
#ifndef NDEBUG
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->is_none_event());
#endif
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    channels_.erase(fd);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void epoll_poller::update(int operation, socket_channel* channel) {
    struct epoll_event event;
    mem_zero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
//   LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
//     << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            // LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
        else {
            // LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
        }
    }
}

} /* namespace speed::mq */