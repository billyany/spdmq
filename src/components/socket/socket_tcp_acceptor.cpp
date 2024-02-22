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

#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>

#include "event_loop.h"
#include "inet_address.h"
#include "socket_tcp_acceptor.h"

namespace speed::mq {

socket_tcp_acceptor::socket_tcp_acceptor(event_loop* loop, const inet_address& listen_addr, bool reuse_port)
    : loop_(loop),
      accept_socket_(create_nonblocking_socket(listen_addr.family())),
      accept_channel_(*loop, accept_socket_.fd()),
      listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idle_fd_ >= 0);
    accept_socket_.set_reuse_addr(true);
    accept_socket_.set_reuse_port(reuse_port);
    accept_socket_.bind_address(listen_addr);
    accept_channel_.set_read_callback(std::bind(&socket_tcp_acceptor::handle_read, this));
}

socket_tcp_acceptor::~socket_tcp_acceptor() {
    accept_channel_.disable_all();
    accept_channel_.remove();
    ::close(idle_fd_);
}

void socket_tcp_acceptor::listen() {
    loop_->assert_in_loop_thread();
    listening_ = true;
    accept_socket_.listen();
    accept_channel_.enable_reading();
}

void socket_tcp_acceptor::handle_read() {
    loop_->assert_in_loop_thread();
    inet_address peer_addr;
    // TODO : 是否考虑循环接收，直到返回 -1 ？
    int connfd = accept_socket_.accept(&peer_addr);
    if (connfd >= 0) {
        std::string host_port = peer_addr.to_ip_port();
        LOG_DEBUG("accepts of %s", host_port.data());
        if (new_connection_callback_) {
            new_connection_callback_(connfd, peer_addr);
        }
        else {
            close(connfd);
        }
    } else {
        LOG_INFO("socket_tcp_acceptor::handle_read failed");

        if (errno == EMFILE) {
            ::close(idle_fd_);
            idle_fd_ = ::accept(accept_socket_.fd(), nullptr, nullptr);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} /* namespace speed::mq */
