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

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "spdmq_uncopyable.h"

namespace speed::mq {

class event_loop;
class event_loop_thread;

class event_loop_thread_pool : spdmq_uncopyable {
private:
    event_loop* base_loop_;
    std::string name_;
    bool started_;
    int num_threads_;
    int next_;
    
    std::vector<event_loop*> loops_;
    std::vector<std::unique_ptr<event_loop_thread>> threads_;

public:
    typedef std::function<void(event_loop*)> thread_init_callback;

    event_loop_thread_pool(event_loop* baseLoop, const std::string& name);
    ~event_loop_thread_pool();
    void set_thread_num(int num_threads) { num_threads_ = num_threads; }
    void start(const thread_init_callback& cb = thread_init_callback());

    /// 调研start后有效，循环调用 event_loop
    event_loop* get_next_loop();

    /// 使用相同的哈希代码，它将始终返回相同的 event_loop
    event_loop* get_loop_for_hash(size_t hash_code);

    std::vector<event_loop*> get_all_loops();

    bool started() const { 
        return started_; 
    }

    const std::string& name() const { 
        return name_; 
    }
};

}  /* speed::mq */
