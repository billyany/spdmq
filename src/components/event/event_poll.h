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



#include <unistd.h>
#include "spdmq_event.h"
#include "spdmq_error.hpp"
#include "spdmq_internal_def.h"

namespace speed::mq {

class event_poll : public spdmq_event {
private:
    fd_t epoll_fd_;
    std::atomic_bool destroy_event_loop_ = false;

public:
    event_poll(spdmq_ctx_t& ctx);
    void event_create() override final;
    void event_build() override final;
    void event_destroy() override final;
    void event_add(fd_t fd) override final;
    void event_del(fd_t fd) override final;

private:
    void event_poll_loop();
};

} /* opendbus*/