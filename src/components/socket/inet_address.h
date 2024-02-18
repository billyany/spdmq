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

#include <string>
#include <netinet/in.h>

namespace speed::mq {

/**
 * @brief sockaddr_in的包装器
 * 
 */
class inet_address {
private:
    union {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };

public:
    static bool resolve(const std::string& hostname, inet_address* result);
    explicit inet_address(const struct sockaddr_in& addr);
    explicit inet_address(const struct sockaddr_in6& addr);
    explicit inet_address(uint16_t port = 0, bool loopback_only = false, bool ipv6 = false);
    explicit inet_address(const std::string& ip, uint16_t port, bool ipv6 = false);

    uint16_t port() const;
    std::string to_ip() const;
    std::string to_ip_port() const;
    void set_scope_id(uint32_t scope_id); // set ipv6 scope_id

    const struct sockaddr* get_sock_addr() const;
    void set_sock_addr_inet6(const struct sockaddr_in6& addr6);

private:
    sa_family_t family() const;
    uint32_t ipv4_net_endian() const;
    uint16_t port_net_endian() const;
    void to_ip(char* buf, size_t size, const struct sockaddr* addr) const;   
    void to_ip_port(char* buf, size_t size, const struct sockaddr* addr) const;

}; /* class inet_address */

}  /* namespace speed::mq */
