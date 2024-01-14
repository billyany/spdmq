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

#include "spdmq_impl.h"

namespace speed::mq {

std::shared_ptr<spdmq> spdmq::new_spdmq(spdmq_ctx& ctx) {
    return std::make_shared<spdmq_impl>(ctx);
}

spdmq_code_t spdmq::bind(const std::string& url) {
    return reinterpret_cast<spdmq_impl*>(this)->bind(url);
}

spdmq_code_t spdmq::connect(const std::string& url) {
    return reinterpret_cast<spdmq_impl*>(this)->connect(url);
}

spdmq_code_t spdmq::send(spdmq_msg& msg) {
    return reinterpret_cast<spdmq_impl*>(this)->send(msg);
}

spdmq_code_t spdmq::recv(spdmq_msg& msg, time_msec_t time_out) {
    return reinterpret_cast<spdmq_impl*>(this)->recv(msg, time_out);
}

spdmq::spdmq() {}

spdmq::~spdmq() {}

} /* namespace openspdmq */