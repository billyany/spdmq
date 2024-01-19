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

#include "mode_factory.h"
#include "mode_publish.h"
#include "mode_subscribe.h"

namespace speed::mq {

mode_factory* mode_factory::instance() {
    static mode_factory impl;
    return &impl;
}

std::shared_ptr<spdmq_mode> mode_factory::create_mode(spdmq_ctx_t& ctx) {
    std::shared_ptr<spdmq_mode> spdmq_mode_ptr;
    switch (ctx.mode()) {
        case COMM_MODE::SPDMQ_PUB:
            spdmq_mode_ptr = std::make_shared<mode_publish>(ctx);
            break;
        case COMM_MODE::SPDMQ_SUB:
            spdmq_mode_ptr = std::make_shared<mode_subscribe>(ctx);
            break;
        case COMM_MODE::SPDMQ_UNKNOW:
            throw std::runtime_error("mode_factory::create_mode: unknown mode");
            break;
    }

    return spdmq_mode_ptr;
}

} /* namespace speed::mq */