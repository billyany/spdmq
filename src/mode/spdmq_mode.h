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

#include "spdmq_def.h"
#include "dispatcher.h"

namespace speed::mq {

class spdmq_mode {
private:
    spdmq_ctx_t& ctx_;
    std::shared_ptr<dispatcher> dispatcher_ptr_;

public:
    std::function<void(spdmq_msg_t&)> on_mode_recv;
    std::function<void(spdmq_msg_t&)> on_mode_online;
    std::function<void(spdmq_msg_t&)> on_mode_offline;

public:
    spdmq_mode(spdmq_ctx& ctx);

    virtual ~spdmq_mode() {}
    virtual spdmq_code_t send(spdmq_msg_t& msg);
    virtual spdmq_code_t recv(spdmq_msg_t& msg, time_msec_t time_out);
    virtual void on_recv(comm_msg_t&& msg);
    virtual void on_online(comm_msg_t&& msg);
    virtual void on_offline(comm_msg_t&& msg);
    virtual void registered();

public:
    void bind();
    void connect();
    void spin(bool background);

public:
    spdmq_ctx_t& ctx() {
        return ctx_;
    }

    std::shared_ptr<dispatcher>& handler() {
        return dispatcher_ptr_;
    }    
};

} /* namespace speed::mq */
