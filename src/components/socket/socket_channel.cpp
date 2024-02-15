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

#include <assert.h>
#include <sys/epoll.h>
#include "socket_channel.h"

namespace speed::mq {

const int socket_channel::kNoneEvent = 0;
const int socket_channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int socket_channel::kWriteEvent = EPOLLOUT;

socket_channel::socket_channel(event_loop& loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      tied_(false),
      event_handling_(false),
      added2loop_(false) {}

socket_channel::~socket_channel() {
    assert(!event_handling_);
    assert(!added2loop_);
    // TODO
    // if (loop_.isInLoopThread()) {
    //     assert(!loop_.hasChannel(this));
    // }
}

void socket_channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void socket_channel::update() {
    added2loop_ = true;
    // TODO
    // loop_->update_channel(this);
}

void socket_channel::remove() {
    assert(is_none_event());
    added2loop_ = false;
    // TODO
    // loop_->removeChannel(this);
}

void socket_channel::handle_event() {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        if (guard) {
            handle_event_with_guard();
        }
    }
    else {
        handle_event_with_guard();
    }
}

void socket_channel::handle_event_with_guard() {
    event_handling_ = true;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN) && close_callback_) {
        close_callback_();
    }

    if ((revents_ & (EPOLLERR)) && error_callback_) {
        error_callback_();
    }

    if ((revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) && read_callback_) {
        read_callback_();
    }

    if ((revents_ & EPOLLOUT) && write_callback_) {
        write_callback_();
    }
    event_handling_ = false;
}

} /* namespace speed::mq */
