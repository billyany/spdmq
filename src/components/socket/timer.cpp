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

#include "timer.h"

namespace speed::mq {

std::atomic_int64_t s_num_created_ = 0;

int64_t timer::num_created() {
    return s_num_created_.fetch_add(1);
}

timer::timer(timer_callback cb, const time_point when, int32_t interval)
    : callback_(std::move(cb)), 
      expiration_(when), 
      interval_(interval), 
      repeat_(interval > 0), 
      sequence_(num_created()) {}

void timer::run() const {
    callback_();
}

bool timer::repeat () const {
    return repeat_;
}

int64_t timer::sequence () const {
    return sequence_;
}

void timer::restart(time_point now) {
    if (repeat_) {
        expiration_ = now + time_duration_ms(interval_);
    }
    else {
        expiration_ = time_point::create_invalid_time_point();
    }
}

time_point timer::expiration () const  {
    return expiration_;
}

} /* namespace speed::mq */
