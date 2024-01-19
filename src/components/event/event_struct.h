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

#include <map>
#include <atomic>
#include <sys/epoll.h>

#include "spdmq_def.h"
#include "spdmq_spinlock.hpp"
#include "spdmq_internal_def.h"


namespace speed::mq {

const std::map<event_mode_t, int32_t> gEventModeMap = {
    {EVENT_MODE::EVENT_POLL_LT, EPOLLIN},
    {EVENT_MODE::EVENT_POLL_ET, EPOLLIN | EPOLLET},
};

enum class EVENT : uint8_t {
    READ = 0,
    WRITE = 1,
    CONNECTING = 2,
    CONNECTED = 3,
    DISCONNECT = 4,
};

enum class EVENT_PRIORITY : uint8_t {
    NORMAL = 0,
    URGENT = 1,
};

template<typename T>
class set_queue {
private:
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
    std::set<T> set_;

public:
    T pop() {
        T value;
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        if (!set_.empty()) {
            value = *set_.begin();    
            set_.erase(set_.begin());
        }
        return value;
    }

    T& pop(T& value) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        if (!set_.empty()) {
            value = *set_.begin();    
            set_.erase(set_.begin());
        }
        return value;
    }

    void push(T& value) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        set_.insert(value);
    }

    bool empty() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        return set_.empty();
    }

    std::size_t size() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        return set_.size();
    }

public:
    set_queue() {}
    ~set_queue() {}
};

} /* namespace speed::mq */