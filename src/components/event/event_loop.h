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
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <functional>
#include "spdmq_uncopyable.h"

//
// Reactor, at most one per thread.

namespace speed::mq {

class socket_channel;
class base_poller;
class TimerQueue;

class event_loop : spdmq_uncopyable {
public:
    typedef std::function<void()> Functor;
    event_loop();
    ~event_loop();  // force out-line dtor, for std::unique_ptr members.

private:
    void abort_not_in_loop_thread();
    void handleRead();  // waked up
    void doPendingFunctors();

    void printActiveChannels() const; // DEBUG

    typedef std::vector<socket_channel*> ChannelList;

    bool looping_; /* atomic */
    std::atomic<bool> quit_;
    bool event_handling_; /* atomic */
    bool calling_pending_functors_; /* atomic */
    int64_t iteration_;
    const std::thread::id thread_id_;

    std::unique_ptr<base_poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    // unlike in TimerQueue, which is an internal class,
    // we don't expose socket_channel to client.
    std::unique_ptr<socket_channel> wakeupChannel_;
    std::any context_;

    // scratch variables
    ChannelList activeChannels_;
    socket_channel* currentActiveChannel_;

    mutable std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;

public:

    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            abort_not_in_loop_thread();
        }
    }


    // 永远循环, 必须在创建对象的同一线程中调用。
    void loop();

    // 退出循环。
    // 这不是100%线程安全的，如果您通过原始指针进行调用，
    // 为了100%的安全性，最好通过 shared_ptr＜event_loop＞ 进行调用。
    void quit();

    int64_t iteration() const { return iteration_; }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(Functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(Functor cb);

    size_t queueSize() const;

//   // timers

//   ///
//   /// Runs callback at 'time'.
//   /// Safe to call from other threads.
//   ///
//   TimerId runAt(Timestamp time, TimerCallback cb);
//   ///
//   /// Runs callback after @c delay seconds.
//   /// Safe to call from other threads.
//   ///
//   TimerId runAfter(double delay, TimerCallback cb);
//   ///
//   /// Runs callback every @c interval seconds.
//   /// Safe to call from other threads.
//   ///
//   TimerId runEvery(double interval, TimerCallback cb);
//   ///
//   /// Cancels the timer.
//   /// Safe to call from other threads.
//   ///
//   void cancel(TimerId timerId);

    // internal usage
    void wakeup();
    void updateChannel(socket_channel* channel);
    void removeChannel(socket_channel* channel);
    bool hasChannel(socket_channel* channel);

    bool is_in_loop_thread() const {
        return thread_id_ == std::this_thread::get_id();
    }

    bool event_handling() const { 
        return event_handling_; 
    }

    void setContext(const std::any& context) {
        context_ = context;
    }

    const std::any& getContext() const {
        return context_;
    }

    std::any get_mutable_context() {
        return context_;
    }

    static event_loop* get_event_loop_of_current_thread();
};

}  /* namespace speed::mq */
