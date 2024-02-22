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

#include <any>
#include <cstdint>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <functional>
#include "spdmq_uncopyable.h"
#include "timer.h"

//
// Reactor, at most one per thread.

namespace speed::mq {

class timer_id;
class base_poller;
class timer_queue;
class socket_channel;

class event_loop : spdmq_uncopyable {
public:
    typedef std::function<void()> functor;
    event_loop();
    ~event_loop();  // force out-line dtor, for std::unique_ptr members.

private:
    typedef std::vector<socket_channel*> channel_list_t;

    bool looping_; /* atomic */
    std::atomic<bool> quit_;
    bool event_handling_; /* atomic */
    bool calling_pending_functors_; /* atomic */
    int64_t iteration_;
    const std::thread::id thread_id_;

    std::unique_ptr<base_poller> poller_;
    std::unique_ptr<timer_queue> timer_queue_;
    int32_t wakeup_fd_;
    // unlike in TimerQueue, which is an internal class,
    // we don't expose socket_channel to client.
    std::unique_ptr<socket_channel> wakeup_channel_;
    std::any context_;

    // scratch variables
    channel_list_t active_channels_;
    socket_channel* current_active_channel_;

    mutable std::mutex mutex_;
    std::vector<functor> pending_functors_;

public:

    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            abort_not_in_loop_thread();
        }
    }


    // 永远循环, 必须在创建对象的同一线程中调用。
    void loop();

    // 退出循环。
    // 这不是 100% 线程安全的，如果您通过原始指针进行调用，
    // 为了 100% 的安全性，最好通过 shared_ptr＜event_loop＞ 进行调用。
    void quit();

    int64_t iteration() const { return iteration_; }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void run_in_loop(functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queue_in_loop(functor cb);

    size_t queue_size() const;

  // timers

  ///
  /// Runs callback at 'time'.
  /// Safe to call from other threads.
  ///
  timer_id run_at(time_point time, timer_callback cb);
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  timer_id run_after(int64_t delay, timer_callback cb);
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  ///
  timer_id run_every(int64_t interval, timer_callback cb);
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(timer_id timer_id);

    // internal usage
    void wakeup();
    void update_channel(socket_channel* channel);
    void remove_channel(socket_channel* channel);
    bool has_channel(socket_channel* channel);

    bool is_in_loop_thread() const {
        return thread_id_ == std::this_thread::get_id();
    }

    bool event_handling() const { 
        return event_handling_; 
    }

    void set_context(const std::any& context) {
        context_ = context;
    }

    const std::any& get_context() const {
        return context_;
    }

    std::any& get_mutable_context() {
        return context_;
    }

    static event_loop* get_event_loop_of_current_thread();

private:
    void abort_not_in_loop_thread();
    void handle_read();  // waked up
    void do_pending_functors();
    void print_active_channels() const; // DEBUG

}; /* class event_loop */

}  /* namespace speed::mq */
