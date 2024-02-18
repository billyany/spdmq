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

#include <cstdint>

namespace speed::mq {

class timer;

class timer_id {
public:
    timer_id() : timer_(nullptr), sequence_(0) {}
    timer_id(timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}
    friend class timer_queue;

private:
    timer* timer_;
    int64_t sequence_;
};

}  /* speed::mq */
