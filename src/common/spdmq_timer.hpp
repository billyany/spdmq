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

#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <condition_variable>

#include "spdmq_uncopyable.h"


namespace speed::mq {

class spdmq_timer: public spdmq_uncopyable {
private:
    std::atomic<bool> start_;
    std::atomic<bool> stop_;
    std::mutex lock_;
    std::condition_variable cv_;

public:
    spdmq_timer() : start_(false), stop_(true) {}

    ~spdmq_timer() {
        expire();
    }

    void start(int interval, const std::function<void()>& task, bool immediate = false) {
        if (start_.load()) { return; }

        start_.store(true);
        stop_.store(false);

        if (immediate) { task(); }

        std::thread([this, interval, task] () {
            while (start_.load()) {
                std::unique_lock<std::mutex> lk(lock_);
                if (cv_.wait_for(lk, std::chrono::milliseconds(interval), [this] { return !start_.load(); })) {
                    break;
                }
                task();
            }

            stop_.store(true);
            cv_.notify_one();
        }).detach();
    }

    void expire() {
        if (stop_.load()) {
            return;
        }

        start_.store(false);
        cv_.notify_one();

        std::unique_lock<std::mutex> lk(lock_);
        cv_.wait(lk, [this] { return stop_.load(); });
    }
};

} /* namespace speed::mq */
