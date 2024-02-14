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

#include "spdmq_def.h"
#include "socket_def.h"
#include "spdmq_internal_def.h"

#include <cstdint>
#include <functional>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <vector>

namespace speed::mq {

class socket_endpoint {
private:
    fd_t socket_fd_;
    sockaddr_un sock_address_un_;
    sockaddr_in sock_address_ipv4_;
    sockaddr_in6 sock_address_ipv6_;
    spdmq_ctx_t& ctx_;

public:
    virtual void open_socket (int32_t domain, int32_t type, int32_t protocol);
    virtual void bind () {}
    virtual void listen () {}
    virtual fd_t accept (fd_t server_fd) { return -1; }
    virtual int32_t connect () { return - 1; }
    // virtual int32_t disconnect () { return - 1; }
    // virtual void start_heart (std::function<void ()> task) {}
    // virtual void stop_heart () {}

    // int32_t read_data(int32_t session_id, comm_header_t& header, std::vector<uint8_t>& data);    
    // int32_t write_data(int32_t session_id, const comm_header_t& header, const std::vector<uint8_t>& data);

public:
    
    spdmq_ctx_t& ctx ();
    fd_t& socket_fd ();
    sockaddr_un& sock_address_un ();
    sockaddr_in& sock_address_ipv4 ();
    sockaddr_in6& sock_address_ipv6 ();
    void resolve_address ();
    void close_socket ();

public:
    socket_endpoint(spdmq_ctx_t& ctx);
    virtual ~socket_endpoint();

// private:
//     int32_t on_read_data(int32_t session_id, uint8_t* buf, int32_t buf_len);
};

} /* namespace speed::mq */
