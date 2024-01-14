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

#include "spdmq.h"
#include "spdmq_internal_def.h"

namespace speed::mq {

class spdmq_impl : public spdmq {
private:
    spdmq_ctx ctx_;
    // std::shared_ptr<DBusBase> dbus_ptr_;

public:
    spdmq_impl(const spdmq_ctx& ctx) : ctx_(ctx) {}

    spdmq_code_t bind(const std::string& url);

    spdmq_code_t connect(const std::string& url);

    spdmq_code_t send(spdmq_msg& msg);

    spdmq_code_t recv(spdmq_msg& msg, time_msec_t time_out);

private:
    spdmq_config_t url_format_check_and_parse(const std::string& url);
};

} /* speed::mq */