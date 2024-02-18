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

#include "spdmq_uncopyable.h"

struct tcp_info;

namespace speed::mq {

class inet_address;

/**
 * @brief 套接字文件描述符的包装器
 *        析构时关闭 sockfd, 它是线程安全的, 所有操作都调用系统接口。
 */
class base_socket : spdmq_uncopyable {
private:
    const int sockfd_;

public:
    explicit base_socket(int sockfd);
    ~base_socket();

    /**
    * @brief 获取文件描述符
    *
    */
    int fd() const;
    
    /**
    * @brief 获取tcp信息
    *
    */
    bool get_tcp_info(struct tcp_info*) const;

    /**
    * @brief 获取tcp信息并转换成字符串返回
    *
    */
    bool get_tcp_info_string(char* buf, int len) const;

    /**
    * @brief abort if address in use
    *
    */
    void bind_address(const inet_address& localaddr);

    /**
    * @brief abort if address in use
    *
    */
    void listen();

    /**
    * @brief 成功时，返回一个非负整数
    *        已接受套接字的描述符
    *        设置 SOCK_NONBLOCK | SOCK_CLOEXEC, 并且赋值 *peeraddr。
    *        出现错误时，返回-1，并且不赋值 *peeraddr。
    *
    */
    int accept(inet_address* peeraddr);

    /**
    * @brief 关闭写端口
    *
    */
    void shutdown_write();

    /**
    * @brief Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm)
    *
    */
    void set_tcp_no_delay(bool on);

    /**
    * @brief Enable/disable SO_REUSEADDR
    *
    */
    void set_reuse_addr(bool on);

    /**
    * @brief Enable/disable SO_REUSEPORT
    *
    */
    void set_reuse_port(bool on);

    /**
    * @brief Enable/disable SO_KEEPALIVE
    *
    */
    void set_keep_alive(bool on);
};

}  /* speed::mq */
