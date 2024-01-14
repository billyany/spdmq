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
#include "spdmq_mode_base.h"
#include "spdmq_func.hpp"

namespace speed::mq {

spdmq_code_t spdmq_impl::bind(const std::string& url) {
    auto config = url_format_check_and_parse(url);
    if (!config.parse_result) {
        return SPDMQ_CODE_ADDRESS_ERROR;
    }

    // dbus_ptr_ = DBUS_MODE_CREATE(ctx_, gDBusNetProtocolMap.at(url.substr(0, 3)));
    // if (gDBusNetProtocolMap.at(url.substr(0, 3)) == DBUS_NET_PROTOCOL::IPC) {
    //     return dbus_ptr_->bind("/tmp/" + url.substr(6));
    // }
    // return dbus_ptr_->bind(url.substr(6));
}

spdmq_code_t spdmq_impl::connect(const std::string& url) {
    auto config = url_format_check_and_parse(url);
    if (!config.parse_result) {
        return SPDMQ_CODE_ADDRESS_ERROR;
    }
    // dbus_ptr_ = DBUS_MODE_CREATE(ctx_, gDBusNetProtocolMap.at(url.substr(0, 3)));
    // if (gDBusNetProtocolMap.at(url.substr(0, 3)) == DBUS_NET_PROTOCOL::IPC) {
    //     return dbus_ptr_->connect("/tmp/" + url.substr(6));    
    // }
    // return dbus_ptr_->connect(url.substr(6));
}

spdmq_code_t spdmq_impl::send(spdmq_msg& msg) {
    // return dbus_ptr_->send(dbusData);
}

spdmq_code_t spdmq_impl::recv(spdmq_msg& msg, time_msec_t time_out) {
    // return dbus_ptr_->recv(dbusData, time_out);
}

spdmq_config_t spdmq_impl::url_format_check_and_parse(const std::string& url) {
    spdmq_config_t config = {};
    config.parse_result = true;
    if (url.substr(0, 6) == "ipc://" && regex_match(url.substr(6), R"([a-zA-Z0-9@\._]+)")) {
        config.address = "/tmp/" + url.substr(6);
        ctx_.domain(COMM_DOMAIN::IPC);
    }
    else if ((url.substr(0, 6) == "tcp://" || url.substr(0, 6) == "udp://") && regex_match(url.substr(6), R"(((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}:[0-9]{1,5})")) {
        config.address = url.substr(6);
        config.ip = config.address.substr(0, config.address.find(":"));
        config.port = std::atoi(config.address.substr(config.address.find(":") + 1).c_str());
        ctx_.domain(COMM_DOMAIN::IPV4);
        if (url.substr(0, 3) == "tcp") {
            ctx_.protocol(COMM_PROTOCOL::TCP);
        }
        if (url.substr(0, 3) == "udp") {
            ctx_.protocol(COMM_PROTOCOL::UDP);
        }
    }
    else {
        config.parse_result = false;
    }

    return config;
}

} /* speed::mq */