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
#include "event_poll.h"
#include "event_factory.h"

namespace speed::mq {

event_factory* event_factory::instance() {
    static event_factory impl;
    return &impl;
}

std::shared_ptr<spdmq_event> event_factory::create_event(spdmq_ctx_t& ctx) {
    std::shared_ptr<spdmq_event> event_ptr;
    switch (ctx.event_mode()) {
        case EVENT_MODE::EVENT_POLL_LT:
        case EVENT_MODE::EVENT_POLL_ET:
            event_ptr = std::make_shared<event_poll>(ctx);
            break;
    }
    return event_ptr;
}

} /* namespace speed::mq */