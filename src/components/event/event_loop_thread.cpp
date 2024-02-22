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

#include <assert.h>
#include "event_loop.h"
#include "event_loop_thread.h"

namespace speed::mq {

event_loop_thread::event_loop_thread(const thread_init_callback& cb, const std::string& name)
    : exiting_(false),
      name_(name),
      loop_(nullptr),
      callback_(cb) {
}

event_loop_thread::~event_loop_thread() {
    exiting_ = true;
    if (loop_ != nullptr) { // 不是100%无争用，例如 thread_func 可能正在运行回调
        // 如果 thread_func 刚刚退出，调用析构函数对象的机会仍然很小。
        // 但是当 event_loop_thread 破坏时，通常编程无论如何都会退出。
        loop_->quit();
        thread_.join();
    }
}

event_loop* event_loop_thread::start_loop() {
    thread_ = std::thread(std::bind(&event_loop_thread::thread_func, this), name_);
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return loop_ != nullptr; });
    return loop_;
}

void event_loop_thread::thread_func() {
    event_loop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }

    loop.loop();
    assert(exiting_);
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

} /* namespace speed::mq */
