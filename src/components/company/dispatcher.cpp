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

#include "dispatcher.h"
#include "socket_factory.h"
#include "event_factory.h"

namespace speed::mq {

dispatcher::dispatcher(spdmq_ctx_t& ctx) : ctx_(ctx) {}

void dispatcher::registered_company(spdmq_ctx_t& ctx) {
    // Create socket ptr
    auto comm_mode = ctx.mode();
    if (comm_mode == COMM_MODE::SPDMQ_PUB) {
        spdmq_socket_ptr_ = server_factory::instance()->create_socket(ctx);
    }
    else if (comm_mode == COMM_MODE::SPDMQ_SUB) {
        spdmq_socket_ptr_ = client_factory::instance()->create_socket(ctx);
    }
    else {
        throw std::runtime_error("Unsupported communication mode");
    }
    spdmq_socket_ptr_->open_socket();

    // Create event ptr
    spdmq_event_ptr_ = event_factory::instance()->create_event(ctx);
    spdmq_event_ptr_->event_create();
    spdmq_event_ptr_->event_build();
    // sleep_ms(100);

    // Create storeroom ptr
    storeroom_ptr_ = std::make_shared<storeroom>(ctx);

    // Create porter ptr
    porter_ptr_ = std::make_shared<porter>(ctx, spdmq_event_ptr_, spdmq_socket_ptr_, storeroom_ptr_);

    // Set event callback
    spdmq_event_ptr_->on_read = [this](auto&& T) {
        porter_ptr_->on_read(std::forward<decltype(T)>(T));
    };

    spdmq_event_ptr_->on_connecting = [this](auto&& T) {
        porter_ptr_->on_connecting(std::forward<decltype(T)>(T));
    };

    spdmq_event_ptr_->on_connected = [this](auto&& T) {
        porter_ptr_->on_connected(std::forward<decltype(T)>(T));
    };

    spdmq_event_ptr_->on_disconnect = [this](auto&& T) {
        porter_ptr_->on_disconnect(std::forward<decltype(T)>(T));
    };
}

void dispatcher::bind_company(spdmq_ctx_t& ctx) {
    spdmq_socket_ptr_->resolve_address();
    spdmq_socket_ptr_->bind();
    spdmq_socket_ptr_->listen();

    // Add socket fd to event loop
    spdmq_event_ptr_->event_add(spdmq_socket_ptr_->socket_fd());
}

void dispatcher::connect_company(spdmq_ctx_t& ctx) {
    spdmq_socket_ptr_->resolve_address();
    porter_ptr_->on_reconnect();
}

void dispatcher::operating_company(bool background) {
    spdmq_event_ptr_->event_run(background);
}

} /* namespace speed::mq */
