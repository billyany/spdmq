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
#include <cstdint>
#include <mutex>
#include <unistd.h>
#include <sys/eventfd.h>

#include "event_loop.h"
#include "base_poller.h"
#include "socket_channel.h"
#include "time_point.h"
#include "timer_id.h"
#include "timer_queue.h"

namespace speed::mq {

const int kPollTimeMs = 10000;

__thread event_loop* t_loop_in_this_thread = 0;

int create_eventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        abort();
    }
    return evtfd;
}

event_loop* event_loop::get_event_loop_of_current_thread() {
  return t_loop_in_this_thread;
}

event_loop::event_loop()
  : looping_(false),
    quit_(false),
    event_handling_(false),
    calling_pending_functors_(false),
    iteration_(0),
    thread_id_(std::this_thread::get_id()),
    poller_(base_poller::new_poller(*this)),
    timer_queue_(new timer_queue(this)),
    wakeup_fd_(create_eventfd()),
    wakeup_channel_(new socket_channel(*this, wakeup_fd_)),
    current_active_channel_(nullptr) {
    // LOG_DEBUG << "event_loop created " << this << " in thread " << thread_id_;
    if (t_loop_in_this_thread) {
        // LOG_FATAL << "Another event_loop " << t_loop_in_this_thread << " exists in this thread " << thread_id_;
    } else {
        t_loop_in_this_thread = this;
    }
    wakeup_channel_->set_read_callback(std::bind(&event_loop::handle_read, this));
    wakeup_channel_->enable_reading();
}

event_loop::~event_loop() {
    // LOG_DEBUG << "event_loop " << this << " of thread " << thread_id_
    // << " destructs in thread " << CurrentThread::tid();
    wakeup_channel_->disable_all();
    wakeup_channel_->remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = nullptr;
}

void event_loop::loop() {
    assert(!looping_);
    assert_in_loop_thread();
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
    // LOG_TRACE << "event_loop " << this << " start looping";

    while (!quit_) {
        active_channels_.clear();
        poller_->poll(kPollTimeMs, &active_channels_);
        ++iteration_;
        // if (Logger::logLevel() <= Logger::TRACE) {
        //     printActiveChannels();
        // }
        // TODO sort channel by priority
        event_handling_ = true;
        for (socket_channel* channel : active_channels_) {
            current_active_channel_ = channel;
            current_active_channel_->handle_event();
        }
        current_active_channel_ = nullptr;
        event_handling_ = false;
        do_pending_functors();
    }

    // LOG_TRACE << "event_loop " << this << " stop looping";
    looping_ = false;
}

void event_loop::quit() {
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then event_loop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!is_in_loop_thread()) {
        wakeup();
    }
}

void event_loop::run_in_loop(functor cb) {
    if (is_in_loop_thread()) {
        cb();
    } else {
        queue_in_loop(std::move(cb));
    }
}

void event_loop::queue_in_loop(functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_functors_.push_back(std::move(cb));
    }

    if (!is_in_loop_thread() || calling_pending_functors_) {
        wakeup();
    }
}

size_t event_loop::queue_size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_functors_.size();
}

timer_id event_loop::run_at(time_point time, timer_callback cb) {
    return timer_queue_->add_timer(std::move(cb), time, 0.0);
}

timer_id event_loop::run_after(int64_t delay, timer_callback cb) {
    time_point time(add_time(time_point::now(), delay));
    return run_at(time, std::move(cb));
}

timer_id event_loop::run_every(int64_t interval, timer_callback cb) {
  time_point time(add_time(time_point::now(), interval));
  return timer_queue_->add_timer(std::move(cb), time, interval);
}

void event_loop::cancel(timer_id timer_id) {
    return timer_queue_->cancel(timer_id);
}

void event_loop::update_channel(socket_channel* channel) {
    assert(&channel->owner_loop() == this);
    assert_in_loop_thread();
    poller_->update_channel(channel);
}

void event_loop::remove_channel(socket_channel* channel) {
    assert(&channel->owner_loop() == this);
    assert_in_loop_thread();
    if (event_handling_) {
        assert(current_active_channel_ == channel || 
        std::find(active_channels_.begin(), active_channels_.end(), channel) == active_channels_.end());
    }
    poller_->remove_channel(channel);
}

bool event_loop::has_channel(socket_channel* channel) {
    assert(&channel->owner_loop() == this);
    assert_in_loop_thread();
    return poller_->has_channel(channel);
}

// void event_loop::abortNotInLoopThread()
// {
//   LOG_FATAL << "event_loop::abortNotInLoopThread - event_loop " << this
//             << " was created in thread_id_ = " << thread_id_
//             << ", current thread id = " <<  CurrentThread::tid();
// }

void event_loop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        // LOG_ERROR << "event_loop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void event_loop::handle_read() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
    if (n != sizeof one) {
        // LOG_ERROR << "event_loop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void event_loop::do_pending_functors() {
    std::vector<functor> functors;
    calling_pending_functors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (const functor& fun : functors) {
        fun();
    }
    calling_pending_functors_ = false;
}

// void event_loop::printActiveChannels() const
// {
//   for (const Channel* channel : active_channels_)
//   {
//     LOG_TRACE << "{" << channel->reventsToString() << "} ";
//   }
// }

}  // namespace