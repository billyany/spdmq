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

#include <queue>
#include "spdmq_spinlock.hpp"

/**
 * @brief This is a queue that uses atomic locks and is thread safe.
 *
 */

namespace speed::mq {

template<typename T>
class spdmq_queue {
private:
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
    std::queue<T> queue_;

public:
    void front(T& value) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        value = queue_.front();
    }

    T& front() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        return queue_.front();
    }

    void push(const T& data) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        queue_.emplace(data);
    }

    void push(T&& data) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        queue_.emplace(std::forward<T>(data));
    }

    T pop() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        T value;
        if (!queue_.empty()) {
            value = queue_.front();
            queue_.pop();
        }
        return value;
    }

    T& pop(T& value) {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        if (!queue_.empty()) {
            value = std::move(queue_.front());
            queue_.pop();
        }
        return value;
    }

    bool empty() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        return queue_.empty();
    }

    std::size_t size() {
        spdmq_spinlock<std::atomic_flag> lk(lock_);
        return queue_.size();
    }

public:
    spdmq_queue() {}
    ~spdmq_queue() {}
};

} /* speed::mq */
