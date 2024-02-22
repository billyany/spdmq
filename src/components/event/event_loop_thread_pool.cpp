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
#include "event_loop_thread_pool.h"

namespace speed::mq {

event_loop_thread_pool::event_loop_thread_pool(event_loop* baseLoop, const std::string& name)
    : base_loop_(baseLoop),
      name_(name),
      started_(false),
      num_threads_(0),
      next_(0) {
}

event_loop_thread_pool::~event_loop_thread_pool() {
    // 不要删除 loop，它是堆栈变量
}

void event_loop_thread_pool::start(const thread_init_callback& cb) {
    assert(!started_);
    base_loop_->assert_in_loop_thread();
    started_ = true;

    for (int i = 0; i < num_threads_; ++i) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        event_loop_thread* t = new event_loop_thread(cb, buf);
        threads_.push_back(std::unique_ptr<event_loop_thread>(t));
        loops_.push_back(t->start_loop());
    }
    if (num_threads_ == 0 && cb) {
        cb(base_loop_);
    }
}

event_loop* event_loop_thread_pool::get_next_loop() {
  base_loop_->assert_in_loop_thread();
  assert(started_);
  event_loop* loop = base_loop_;

    if (!loops_.empty()) {
        // round-robin
        loop = loops_[next_++];
        if (static_cast<size_t>(next_) >= loops_.size()) {
            next_ = 0;
        }
    }
    return loop;
}

event_loop* event_loop_thread_pool::get_loop_for_hash(size_t hash_code) {
    base_loop_->assert_in_loop_thread();
    event_loop* loop = base_loop_;

    if (!loops_.empty()) {
        loop = loops_[hash_code % loops_.size()];
    }
    return loop;
}

std::vector<event_loop*> event_loop_thread_pool::get_all_loops() {
    base_loop_->assert_in_loop_thread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<event_loop*>(1, base_loop_);
    }
    else {
        return loops_;
    }
}

} /* namespace speed::mq */
