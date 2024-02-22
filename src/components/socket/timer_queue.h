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

#include <set>
#include <vector>
#include "timer.h"
#include "socket_channel.h"
#include "spdmq_uncopyable.h"

namespace speed::mq {

class event_loop;
class timer_id;

// 尽力而为计时器队列,不能保证回调会准时。
class timer_queue : spdmq_uncopyable {
private:
    typedef std::pair<time_point, timer*> entry;
    typedef std::set<entry> timer_list;
    typedef std::pair<timer*, int64_t> active_timer;
    typedef std::set<active_timer> active_timer_set;

    event_loop* loop_;
    const int timerfd_;
    socket_channel timerfd_channel_;
    // timer list sorted by expiration
    timer_list timers_;

    bool calling_expired_timers_;
    active_timer_set active_timers_;
    active_timer_set canceling_timers_;

public:
    explicit timer_queue(event_loop* loop);
    ~timer_queue();
    timer_id add_timer(timer_callback cb, time_point when, int32_t interval);
    void cancel(timer_id timer_id);

private:
    void add_timer_in_loop(timer* timer);
    void cancel_in_loop(timer_id timerId);
    void handle_read();

    // move out all expired timers
    std::vector<entry> get_expired(time_point now);
    void reset(const std::vector<entry>& expired, time_point now);
    bool insert(timer* timer);
};

}  /* namespace speed::mq */
