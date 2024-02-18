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

#include <atomic>
#include <chrono>
#include <functional>
#include "spdmq_uncopyable.h"

namespace speed::mq {

typedef std::function<void()> timer_callback;
typedef std::chrono::time_point<std::chrono::system_clock> time_stamp;

class timer : spdmq_uncopyable {
private:
    static std::atomic_int64_t s_num_created_;
    const timer_callback callback_;
    time_stamp expiration_;
    const int32_t interval_;
    const bool repeat_;
    const int64_t sequence_;

public:
    static int64_t num_created();
    timer(timer_callback cb, const time_stamp when, int32_t interval);
    void run() const;
    bool repeat () const;
    int64_t sequence () const;
    void restart(time_stamp now);
    time_stamp expiration () const;
};

}  /* speed::mq */
