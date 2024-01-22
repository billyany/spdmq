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
#include "mode_factory.h"
#include "spdmq_func.hpp"

namespace speed::mq {

spdmq_impl::spdmq_impl(spdmq_ctx& ctx) : ctx_(ctx) {
    spdmq_mode_ptr_ = mode_factory::instance()->create_mode(ctx, on_recv, on_online, on_offline);
}

spdmq_code_t spdmq_impl::bind(const std::string& url) {
    auto url_parse = url_format_check_and_parse(url);
    if (!url_parse.parse_result) {
        return SPDMQ_CODE_ADDRESS_ERROR;
    }
    spdmq_mode_ptr_->bind();
    return SPDMQ_CODE_OK;
}

spdmq_code_t spdmq_impl::connect(const std::string& url) {
    auto url_parse = url_format_check_and_parse(url);
    if (!url_parse.parse_result) {
        return SPDMQ_CODE_ADDRESS_ERROR;
    }
    spdmq_mode_ptr_->connect();
    return SPDMQ_CODE_OK;
}

spdmq_code_t spdmq_impl::send(spdmq_msg_t& msg) {
    return spdmq_mode_ptr_->send(msg);
}

spdmq_code_t spdmq_impl::recv(spdmq_msg_t& msg, time_msec_t time_out) {
    return spdmq_mode_ptr_->recv(msg, time_out);
}

void spdmq_impl::spin(bool background) {
    return spdmq_mode_ptr_->spin(background);
}

spdmq_url_parse_t spdmq_impl::url_format_check_and_parse(const std::string& url) {
    spdmq_url_parse_t url_parse = {};
    url_parse.parse_result = true;
    if (url.substr(0, 6) == "ipc://" && regex_match(url.substr(6), R"([a-zA-Z0-9@\._]+)")) {
        url_parse.address = "/tmp/" + url.substr(6);
        ctx_.domain(COMM_DOMAIN::IPC);
        ctx_.config<socket_mode_t>("socket_mode", SOCKET_MODE::UDS);
    }
    else if ((url.substr(0, 6) == "tcp://" || url.substr(0, 6) == "udp://") && regex_match(url.substr(6), R"(((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}:[0-9]{1,5})")) {
        url_parse.address = url.substr(6);
        url_parse.ip = url_parse.address.substr(0, url_parse.address.find(":"));
        url_parse.port = std::atoi(url_parse.address.substr(url_parse.address.find(":") + 1).c_str());
        ctx_.domain(COMM_DOMAIN::IPV4);
        if (url.substr(0, 3) == "tcp") {
            ctx_.protocol_type(COMM_PROTOCOL_TYPE::TCP);
            ctx_.config<socket_mode_t>("socket_mode", SOCKET_MODE::TCP);
        }
        if (url.substr(0, 3) == "udp") {
            ctx_.protocol_type(COMM_PROTOCOL_TYPE::UDP);
            ctx_.config<socket_mode_t>("socket_mode", SOCKET_MODE::UDP);
        }
    }
    else {
        url_parse.parse_result = false;
    }
    ctx_.config<spdmq_url_parse_t>("url_parse", url_parse);
    return url_parse;
}

} /* namespace speed::mq */