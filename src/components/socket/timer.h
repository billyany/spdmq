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
#include <functional>

#include "time_point.h"
#include "spdmq_uncopyable.h"

namespace speed::mq {

typedef std::function<void()> timer_callback;

class timer : spdmq_uncopyable {
private:
    static std::atomic_int64_t s_num_created_;

private:
    const timer_callback callback_;
    time_point expiration_;
    const int32_t interval_;
    const bool repeat_;
    const int64_t sequence_;

public:
    static int64_t num_created();

public:
    timer(timer_callback cb, const time_point when, int32_t interval);

    void run() const;
    bool repeat () const;
    int64_t sequence () const;
    void restart(time_point now);
    time_point expiration () const;
};

}  /* speed::mq */
