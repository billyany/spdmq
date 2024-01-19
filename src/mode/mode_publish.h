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

#include <cstdint>
#include "spdmq_mode.h"

namespace speed::mq {

class mode_publish : public spdmq_mode {
private:
    std::map<std::string, std::set<int32_t>> subscribe_table_; // subscription topic table
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;

public:
    mode_publish(spdmq_ctx& ctx);

    void registered() override;

    spdmq_code_t send(spdmq_msg_t& msg) override;

    void on_recv(comm_msg_t&& msg) override;

    void on_offline(comm_msg_t&& msg) override;

private:
    void msg_deal(const comm_msg_t& msg);
    void topic_insert(fd_t session_id, const std::string& topic);
    void session_remove(fd_t session_id);
};

} /* namespace speed::mq */