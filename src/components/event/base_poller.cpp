// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "epoll_poller.h"
#include "event_loop.h"
#include "base_poller.h"
#include "socket_channel.h"

namespace speed::mq {

base_poller::base_poller(event_loop& loop)
    : owner_loop_(loop) {}
base_poller::~base_poller() = default;

bool base_poller::has_channel(socket_channel* channel) const {
    assert_in_loop_thread();
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

void base_poller::assert_in_loop_thread() const {
    owner_loop_.assert_in_loop_thread();
}

base_poller* base_poller::new_poller(event_loop& loop) {
    return new epoll_poller(loop);
}

} /* namespace speed::mq */