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

#include "spdmq_def.h"
#include <memory>
#include <functional>

#define NEW_SPDMQ speed::mq::spdmq::new_spdmq

namespace speed::mq {

class spdmq {
public:
    /**
     * @brief create spdmq object
     *
     * @param ctx [input]: context of spdmq object
     * 
     * @return spdmq smart shared ptr
     * 
     * @note spdmq objects can only pass through new_spdmq static member function creation
     */
    static std::shared_ptr<spdmq> new_spdmq(spdmq_ctx& ctx);

    /**
     * @brief bind service address
     *
     * @param url [input]: Service address, format as follows:
     *              tcp://ip:port
     *              ipc://[a-zA-Z0-9@\._]+
     * 
     * @return SPDMQ_OK - bind success
     * 
     * @note for details on "spdmq_code_t", please refer to the "spdmq_def. h" header file
     *
     */
    spdmq_code_t bind(const std::string& url);

    /**
     * @brief connect service address
     *
     * @param url [input]: Service address, format as follows:
     *              tcp://ip:port
     *              ipc://[a-zA-Z0-9@\._]+
     * 
     * @return SPDMQ_OK - connect success
     * 
     * @note for details on "spdmq_code_t", please refer to the "spdmq_def. h" header file
     *
     */
    spdmq_code_t connect(const std::string& url);

    /**
     * @brief send msg
     * 
     * @param msg [input]: send msg
     * 
     * @return SPDMQ_OK - send success
     * 
     * @note for details on "spdmq_code_t", please refer to the "spdmq_def. h" header file
     *
     */
    spdmq_code_t send(spdmq_msg_t& msg);

    /**
     * @brief receive data (if the "on_recv" callback function is used, data cannot be obtained through "recv" function)
     * 
     * @param msg [output]: recv msg
     * 
     * @param time_out [input]: equal to 0 never timeout, greater than 0 indicates timeout time (unit millisecond)
     *
     * @return SPDMQ_OK - recv success
     * 
     * @note for details on "spdmq_code_t", please refer to the "spdmq_def. h" header file
     *
     */
    spdmq_code_t recv(spdmq_msg_t& msg, time_msec_t time_out = 0);

    void spin(bool background = false);

public:
    /**
     * @brief receive message callback function object
     * 
     */
    std::function<void(spdmq_msg_t&)> on_recv;

    /**
     * @brief connect callback function object
     * 
     */
    std::function<void(spdmq_msg_t&)> on_online;

    /**
     * @brief disconnect callback function object
     * 
     */
    std::function<void(spdmq_msg_t&)> on_offline;

protected:
    spdmq();
    ~spdmq();
};

} /* namespace speed::mq */
