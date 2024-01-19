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
#include "spdmq_def.h"
#include "spdmq_socket.h"

namespace speed::mq {

class socket_factory {
public:
    virtual std::shared_ptr<spdmq_socket> create_socket(spdmq_ctx_t& ctx) = 0;
    virtual ~socket_factory() {}
};

class server_factory : public socket_factory {
public:
    static server_factory* instance();
    std::shared_ptr<spdmq_socket> create_socket(spdmq_ctx_t& ctx) override;
};

class client_factory : public socket_factory {
public:
    static client_factory* instance();
    std::shared_ptr<spdmq_socket> create_socket(spdmq_ctx_t& ctx) override;
};

} /* namespace speed::mq */
