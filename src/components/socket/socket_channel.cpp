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

#include "event_loop.h"
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
    if (loop_.is_in_loop_thread()) {
        assert(!loop_.has_channel(this));
    }
}

void socket_channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void socket_channel::update() {
    added2loop_ = true;
    loop_.update_channel(this);
}

void socket_channel::remove() {
    assert(is_none_event());
    added2loop_ = false;
    loop_.remove_channel(this);
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

void socket_channel::set_read_callback(read_event_callback cb) {
    read_callback_ = std::move(cb);
}

void socket_channel::set_write_callback(event_callback cb) {
    write_callback_ = std::move(cb);
}

void socket_channel::set_close_callback(event_callback cb) { 
    close_callback_ = std::move(cb);
}

void socket_channel::set_error_callback(event_callback cb) {
    error_callback_ = std::move(cb);
}

int32_t socket_channel::fd() const {
    return fd_;
}

int32_t socket_channel::events() const {
    return events_;
}

void socket_channel::set_revents(int32_t revt) {  // used by pollers
    revents_ = revt;
}

bool socket_channel::is_none_event() const {
        return events_ == kNoneEvent;
}

void socket_channel::enable_reading() {
    events_ |= kReadEvent;
    update();
}

void socket_channel::disable_reading() {
    events_ &= ~kReadEvent;
    update();
}

void socket_channel::enable_writing() {
    events_ |= kWriteEvent;
    update();
}

void socket_channel::disable_writing() {
    events_ &= ~kWriteEvent;
    update();
}

void socket_channel::disable_all() {
    events_ = kNoneEvent;
    update();
}

bool socket_channel::is_writing() const {
    return events_ & kWriteEvent;
}

bool socket_channel::is_reading() const {
    return events_ & kReadEvent;
}

int32_t socket_channel::index() {
    return index_;
}

void socket_channel::set_index(int32_t idx) {
    index_ = idx;
}

event_loop& socket_channel::owner_loop() {
    return loop_;
}

} /* namespace speed::mq */
