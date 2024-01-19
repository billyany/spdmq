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


#include "spdmq_socket.h"
#include "porter.h"
#include "storeroom.h"
namespace speed::mq {

class dispatcher {
private:
    spdmq_ctx_t& ctx_;
    
    std::shared_ptr<porter> porter_ptr_;
    std::shared_ptr<storeroom> storeroom_ptr_;
    std::shared_ptr<spdmq_event> spdmq_event_ptr_;
    std::shared_ptr<spdmq_socket> spdmq_socket_ptr_;

public:
    dispatcher(spdmq_ctx_t& ctx);

    void registered_company (spdmq_ctx_t& ctx);
    void destroy_company (spdmq_ctx_t& ctx) {}

    void bind_company (spdmq_ctx_t& ctx);
    void unbind_company (spdmq_ctx_t& ctx) {};

    void connect_company (spdmq_ctx_t& ctx);
    void disconnect_company (spdmq_ctx_t& ctx) {}

    void operating_company (bool background);
    void closure_company (bool background) {}

public:
    std::shared_ptr<porter>& porter_ptr() {
        return porter_ptr_;
    }

    std::shared_ptr<spdmq_socket>& spdmq_socket_ptr() {
        return spdmq_socket_ptr_;
    }

private:
    // void on_connect_company();

private:
    spdmq_ctx_t& ctx() {
        return ctx_;
    }
};

} /* namespace speed::mq */