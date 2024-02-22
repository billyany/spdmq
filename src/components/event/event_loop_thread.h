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
#include <string>
#include <functional>
#include <condition_variable>
#include "spdmq_uncopyable.h"

namespace speed::mq {

class event_loop;

class event_loop_thread : spdmq_uncopyable {
public:
    typedef std::function<void(event_loop*)> thread_init_callback;

    event_loop_thread(const thread_init_callback& cb = thread_init_callback(), const std::string& name = std::string());
    ~event_loop_thread();
    event_loop* start_loop();

private:
    void thread_func();

private:
    bool exiting_;
    std::string name_;
    event_loop* loop_;
    std::mutex mutex_;
    std::thread thread_;
    std::condition_variable cond_;
    thread_init_callback callback_;

}; /* class event_loop_thread */

}  /* namespace speed::mq */
