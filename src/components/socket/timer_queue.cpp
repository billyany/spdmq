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

#include <unistd.h>
#include <sys/timerfd.h>

#include "assert.h"
#include "time_point.h"
#include "timer.h"
#include "timer_queue.h"
#include "event_loop.h"
#include "timer_id.h"
#include "spdmq_logger.hpp"
#include "spdmq_func.hpp"

namespace speed::mq {

int32_t create_timerfd() {
    int32_t timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        LOG_FATAL("Failed in timerfd_create");
    }
    return timerfd;
}

timespec how_much_time_from_now(time_point when) {
    int64_t microseconds = when - time_point::now();
    if (microseconds < 100) {
        microseconds = 100;
    }

    timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / (1000 * 1000));
    ts.tv_nsec = static_cast<long>((microseconds % (1000 * 1000)) * 1000);
    return ts;
}

void read_timerfd(int timerfd, time_point now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_DEBUG("timer_queue::handle_read() %llu, %llu", howmany, now.micros_count());
    if (n != sizeof howmany) {
        LOG_ERROR("timer_queue::handle_read() reads %ld bytes instead of 8", n);
    }
}

void reset_timerfd(int timerfd, time_point expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    mem_zero(&new_value, sizeof new_value);
    mem_zero(&old_value, sizeof old_value);
    new_value.it_value = how_much_time_from_now(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret) {
        LOG_ERROR("timerfd_settime()");
    }
}

timer_queue::timer_queue(event_loop* loop)
    : loop_(loop),
      timerfd_(create_timerfd()),
      timerfd_channel_(*loop, timerfd_),
      timers_(),
      calling_expired_timers_(false) {
    timerfd_channel_.set_read_callback(std::bind(&timer_queue::handle_read, this));
    timerfd_channel_.enable_reading(); // 我们总是读 timerfd，我们用 timerfd_settime 取消读取
}

timer_queue::~timer_queue()
{
    timerfd_channel_.disable_all();
    timerfd_channel_.remove();
    ::close(timerfd_);
    // do not remove channel, since we're in event_loop::dtor();
    for (const entry& timer : timers_) {
        delete timer.second;
    }
}

timer_id timer_queue::add_timer(timer_callback cb, time_point when, int32_t interval) {
    timer* timer = new speed::mq::timer(std::move(cb), when, interval);
    loop_->run_in_loop(std::bind(&timer_queue::add_timer_in_loop, this, timer));
    return timer_id(timer, timer->sequence());
}

void timer_queue::cancel(timer_id timer_id) {
    loop_->run_in_loop(std::bind(&timer_queue::cancel_in_loop, this, timer_id));
}

void timer_queue::add_timer_in_loop(timer* timer) {
    loop_->assert_in_loop_thread();
    bool earliest_changed = insert(timer);

    if (earliest_changed) {
        reset_timerfd(timerfd_, timer->expiration());
    }
}

void timer_queue::cancel_in_loop(timer_id timerId) {
    loop_->assert_in_loop_thread();
    assert(timers_.size() == active_timers_.size());
    active_timer timer(timerId.timer_, timerId.sequence_);
    auto it = active_timers_.find(timer);
    if (it != active_timers_.end()) {
    size_t n = timers_.erase(entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        delete it->first; // FIXME: no delete please
        active_timers_.erase(it);
    }
    else if (calling_expired_timers_) {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void timer_queue::handle_read() {
    loop_->assert_in_loop_thread();
    auto now = time_point::now();
    read_timerfd(timerfd_, now);

    std::vector<entry> expired = get_expired(now);

    calling_expired_timers_ = true;
    canceling_timers_.clear();
    // safe to callback outside critical section
    for (const entry& it : expired) {
        it.second->run();
    }
    calling_expired_timers_ = false;

    reset(expired, now);
}

std::vector<timer_queue::entry> timer_queue::get_expired(time_point now) {

    assert(timers_.size() == active_timers_.size());
    std::vector<entry> expired;
    entry sentry(now, reinterpret_cast<timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || now < end->first);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);

    for (const entry& it : expired) {
        active_timer timer(it.second, it.second->sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == active_timers_.size());

    return expired;
}

void timer_queue::reset(const std::vector<entry>& expired, time_point now) {
    for (const entry& it : expired)
    {
        active_timer timer(it.second, it.second->sequence());
        if (it.second->repeat() && canceling_timers_.find(timer) == canceling_timers_.end()) {
        it.second->restart(now);
            insert(it.second);
        } else {
            // FIXME move to a free list
            delete it.second; // FIXME: no delete please
        }
    }

    auto next_expire = time_point::create_invalid_time_point();
    if (!timers_.empty()) {
        next_expire = timers_.begin()->second->expiration();
    }

    if (next_expire.valid()) {
        reset_timerfd(timerfd_, next_expire);
    }
}

bool timer_queue::insert(timer* timer) {
    loop_->assert_in_loop_thread();
    assert(timers_.size() == active_timers_.size());
    bool earliest_changed = false;
    time_point when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliest_changed = true;
    }
    {
        auto result1 = timers_.insert(entry(when, timer));
        assert(result1.second);
        (void)result1;

        auto result2 = active_timers_.insert(active_timer(timer, timer->sequence()));
        assert(result2.second);
        (void)result2;
    }

    assert(timers_.size() == active_timers_.size());
    return earliest_changed;
}

} /* namespace speed::mq */
