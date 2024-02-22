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

#include <assert.h>
#include <stdio.h>  // snprintf
#include <functional>
#include "event_loop.h"
#include "inet_address.h"
#include "socket_connect.h"
#include "spdmq_logger.hpp"
#include "socket_tcp_server.h"
#include "socket_tcp_acceptor.h"
#include "event_loop_thread_pool.h"
using namespace std::placeholders;

namespace speed::mq {

socket_tcp_server::socket_tcp_server(event_loop* loop, 
                                     const inet_address& listen_addr,
                                     const std::string& name,
                                     Option option)
    : loop_(loop),
      ip_port_(listen_addr.to_ip_port()),
      name_(name),
      acceptor_(new socket_tcp_acceptor(loop, listen_addr, option == kReusePort)),
      thread_pool_(new event_loop_thread_pool(loop, name_)),
      next_conn_id_(1) {
    acceptor_->set_new_connection_callback(std::bind(&socket_tcp_server::new_connection, this, _1, _2));
}

socket_tcp_server::~socket_tcp_server() {
    loop_->assert_in_loop_thread();
    LOG_TRACE("TcpServer::~TcpServer [%s] destructing", name_.data());

    for (auto& item : connections_) {
        tcp_connection_ptr conn(item.second);
        item.second.reset();
        conn->get_loop()->run_in_loop(std::bind(&socket_connect::connect_destroyed, conn));
    }
}

void socket_tcp_server::set_thread_num(int num_threads) {
    assert(0 <= num_threads);
    thread_pool_->set_thread_num(num_threads);
}

void socket_tcp_server::start() {
    if (started_.load() == 0) {
        started_.store(1);
        thread_pool_->start(thread_init_callback_);

        assert(!acceptor_->listening());
        loop_->run_in_loop(std::bind(&socket_tcp_acceptor::listen, acceptor_.get()));
    }
}

void socket_tcp_server::new_connection(int sockfd, const inet_address& peer_addr) {
    loop_->assert_in_loop_thread();
    event_loop* io_loop = thread_pool_->get_next_loop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    LOG_INFO("socket_tcp_server::new_connection [%s] - new connection [%s] from %s", name_.data(), conn_name.data(), peer_addr.to_ip_port().data());
    inet_address local_addr(get_local_addr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    tcp_connection_ptr conn(new socket_connect(io_loop, conn_name, sockfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->set_connection_callback(connection_callback_);
    conn->set_message_callback(message_callback_);
    conn->set_write_complete_callback(write_complete_callback_);
    conn->set_close_callback(std::bind(&socket_tcp_server::remove_connection, this, _1)); // FIXME: unsafe
    io_loop->run_in_loop(std::bind(&socket_connect::connect_established, conn));
}

void socket_tcp_server::remove_connection(const tcp_connection_ptr& conn) {
    // FIXME: unsafe
    loop_->run_in_loop(std::bind(&socket_tcp_server::remove_connection_in_loop, this, conn));
}

void socket_tcp_server::remove_connection_in_loop(const tcp_connection_ptr& conn) {
    loop_->assert_in_loop_thread();
    LOG_INFO("socket_tcp_server::remove_connection_in_loop [%s] - connection %s", name_.data(), conn->name().data());
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    (void)n;
    event_loop* io_loop = conn->get_loop();
    io_loop->queue_in_loop(std::bind(&socket_connect::connect_destroyed, conn));
}

} /* namespace speed::mq */
