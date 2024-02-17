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
#include <functional>
#include "spdmq_uncopyable.h"


/**
* @brief 一个可选的I/O通道，此类不拥有文件描述符, 文件描述符可以是套接字，事件fd、时间rfd或信号fd。
*
*/

namespace speed::mq {

class event_loop;
class socket_channel : spdmq_uncopyable {
public:
    typedef std::function<void()> event_callback;
    typedef std::function<void()> read_event_callback;

private:
    static const int32_t kNoneEvent;
    static const int32_t kReadEvent;
    static const int32_t kWriteEvent;

    event_loop&    loop_;
    const int32_t  fd_;
    int32_t        events_;
    int32_t        revents_; // it's the received event types of epoll or poll
    int32_t        index_;   // used by Poller.

    std::weak_ptr<void> tie_;
    bool tied_;
    bool event_handling_;
    bool added2loop_;
    read_event_callback read_callback_;
    event_callback write_callback_;
    event_callback close_callback_;
    event_callback error_callback_;

 public:
    socket_channel(event_loop& loop, int32_t fd);
    ~socket_channel();

    void handle_event();
    void handle_event_with_guard();

    void set_read_callback(read_event_callback cb);
    void set_write_callback(event_callback cb);
    void set_close_callback(event_callback cb);
    void set_error_callback(event_callback cb);

    // 将此通道绑定到 shared_ ptr 管理的所有者对象，防止所有者对象在 handle_event 中被销毁。
    void tie(const std::shared_ptr<void>&);
    int32_t fd() const;

    int32_t events() const;
    void set_revents(int32_t revt); // used by pollers
    
    bool is_reading() const;
    void enable_reading();
    void disable_reading();

    bool is_writing() const;
    void enable_writing();
    void disable_writing();

    bool is_none_event() const;
    void disable_all();

    // for Poller
    int32_t index();
    void set_index(int32_t idx);
    event_loop& owner_loop();
    void remove();

private:
    void update();
};

}  /* speed::mq */
