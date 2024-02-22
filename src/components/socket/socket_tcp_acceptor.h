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

#pragma  once

#include <functional>

#include "base_socket.h"
#include "socket_channel.h"
#include "spdmq_uncopyable.h"


namespace speed::mq {

class event_loop;
class inet_address;

using new_connection_callback = std::function<void (int sockfd, const inet_address&)>;

class socket_tcp_acceptor : spdmq_uncopyable {
private:
    event_loop* loop_;
    base_socket accept_socket_;
    socket_channel accept_channel_;
    new_connection_callback new_connection_callback_;

    bool listening_;
    int idle_fd_;

public:
    socket_tcp_acceptor(event_loop* loop, const inet_address& listen_addr, bool reuse_port);
    ~socket_tcp_acceptor();

    void set_new_connection_callback(const new_connection_callback& cb) { 
        new_connection_callback_ = cb;
    }

    void listen();

    bool listening() const { return listening_; }

private:
    void handle_read();
};

}  /* namespace spee::mq */
