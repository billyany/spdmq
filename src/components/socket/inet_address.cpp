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


#include <assert.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "inet_endian.h"
#include "inet_address.h"
#include "spdmq_func.hpp"
#include "spdmq_logger.hpp"

// /* Structure describing an internet socket address.  */
// /* internet address. */
// typedef uint32_t in_addr_t;
// struct in_addr {
//     in_addr_t       s_addr;    /* address in network byte order */
// };

// struct sockaddr_in {
//     sa_family_t    sin_family; /* address family: AF_INET */
//     uint16_t       sin_port;   /* port in network byte order */
//     struct in_addr sin_addr;   /* internet address */
// };

// struct sockaddr_in6 {
//     sa_family_t     sin6_family;   /* address family: AF_INET6 */
//     uint16_t        sin6_port;     /* port in network byte order */
//     uint32_t        sin6_flowinfo; /* IPv6 flow information */
//     struct in6_addr sin6_addr;     /* IPv6 address */
//     uint32_t        sin6_scope_id; /* IPv6 scope-id */
// };

namespace speed::mq {

static_assert(sizeof(inet_address) == sizeof(struct sockaddr_in6), "inet_address is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

static __thread char t_resolve_buffer[64 * 1024];
bool inet_address::resolve(const std::string& hostname, inet_address* result) {
    assert(result != nullptr);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    mem_zero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolve_buffer, sizeof t_resolve_buffer, &he, &herrno);
    if (ret == 0 && he != nullptr) {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    } else {
        if (ret) {
            LOG_ERROR("inet_address::resolve");
        }
        return false;
    }
}

inet_address::inet_address(const struct sockaddr_in& addr) : addr_(addr) {}

inet_address::inet_address(const struct sockaddr_in6& addr): addr6_(addr) {}

inet_address::inet_address(uint16_t port, bool loopback_only, bool ipv6) {
    static_assert(offsetof(inet_address, addr6_) == 0, "addr6_ offset 0");
    static_assert(offsetof(inet_address, addr_) == 0, "addr_ offset 0");
    if (ipv6) {
        mem_zero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopback_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = host2network16(port);
    } else {
        mem_zero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopback_only ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = host2network32(ip);
        addr_.sin_port = host2network16(port);
    }
}

inet_address::inet_address(const std::string& ip, uint16_t port, bool ipv6) {
    if (ipv6 || strchr(ip.c_str(), ':')) {
        mem_zero(&addr6_, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = host2network16(port);
        if (::inet_pton(AF_INET6, ip.data(), &addr6_.sin6_addr) <= 0) {
            LOG_ERROR("inet_pton ipv6");
        }
    } else {
        mem_zero(&addr_, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = host2network16(port);
        if (::inet_pton(AF_INET, ip.data(), &addr_.sin_addr) <= 0) {
            LOG_ERROR("inet_pton ipv4");
        }
    }
}

uint16_t inet_address::port() const {
    return network2host16(port_net_endian());
}

void inet_address::set_scope_id(uint32_t scope_id) {
    if (family() == AF_INET6) {
        addr6_.sin6_scope_id = scope_id;
    }
}

std::string inet_address::to_ip() const {
    char buf[64] = "";
    to_ip(buf, sizeof buf, get_sock_addr());
    return buf;
}

std::string inet_address::to_ip_port() const {
    char buf[64] = "";
    to_ip_port(buf, sizeof buf, get_sock_addr());
    return buf;
}

const sockaddr* inet_address::get_sock_addr() const {
    return reinterpret_cast<const sockaddr*>((&addr6_));
}

void inet_address::set_sock_addr_inet6(const sockaddr_in6& addr6) {
    addr6_ = addr6;
}

sa_family_t inet_address::family() const { 
    return addr_.sin_family;
}

uint32_t inet_address::ipv4_net_endian() const {
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t inet_address::port_net_endian() const {
    return addr_.sin_port;
}

void inet_address::to_ip(char* buf, size_t size, const struct sockaddr* addr) const {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void inet_address::to_ip_port(char* buf, size_t size, const struct sockaddr* addr) const {
    if (addr->sa_family == AF_INET6) {
        buf[0] = '[';
        to_ip(buf + 1, size - 1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6* addr6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
        uint16_t port = network2host16(addr6->sin6_port);
        assert(size > end);
        snprintf(buf + end, size-end, "]:%u", port);
        return;
    }

    to_ip(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = reinterpret_cast<const struct sockaddr_in*>(addr);
    uint16_t port = network2host16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

} /* namespace speed::mq */
