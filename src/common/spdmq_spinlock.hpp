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

namespace speed::mq {

template<typename T>
class spdmq_spinlock {
private:
    T* lock_;

public:
    explicit spdmq_spinlock(T& lock) : lock_(std::addressof(lock))
    {
        while (lock_->test_and_set(std::memory_order_acquire));
    }

    ~spdmq_spinlock()
    {
        lock_->clear(std::memory_order_release);
    }
};

} /* namespace speed::mq */
