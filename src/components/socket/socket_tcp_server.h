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

#include <map>
#include <atomic>
#include "socket_connect.h"
#include "spdmq_uncopyable.h"

namespace speed::mq {

class event_loop;
class inet_address;
class socket_tcp_acceptor;
class event_loop_thread_pool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.

typedef std::function<void(event_loop*)> thread_init_callback;

class socket_tcp_server : spdmq_uncopyable {
private:
    typedef std::map<std::string, tcp_connection_ptr> connection_map;

    event_loop* loop_; // the acceptor loop
    const std::string ip_port_;
    const std::string name_;
    std::unique_ptr<socket_tcp_acceptor> acceptor_; // avoid revealing acceptor
    std::shared_ptr<event_loop_thread_pool> thread_pool_;
    connection_callback connection_callback_;
    message_callback message_callback_;
    write_complete_callback write_complete_callback_;
    thread_init_callback thread_init_callback_;
    std::atomic_int32_t started_ = 0;
    // always in loop thread
    int next_conn_id_;
    connection_map connections_;

public:
    enum Option {
        kNoReusePort,
        kReusePort,
    };

    socket_tcp_server(event_loop* loop, 
                      const inet_address& listenAddr,
                      const std::string& name,
                      Option option = kNoReusePort);
    ~socket_tcp_server();  // force out-line dtor, for std::unique_ptr members.

    const std::string& ip_port() const { return ip_port_; }
    const std::string& name() const { return name_; }
    event_loop* get_loop() const { return loop_; }

    /// Set the number of threads for handling input.
    ///
    /// Always accepts new connection in loop's thread.
    /// Must be called before @c start
    /// @param numThreads
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.
    void set_thread_num(int num_threads);
    void set_thread_init_callback(const thread_init_callback& cb) { threadInitCallback_ = cb; }
    /// valid after calling start()
    std::shared_ptr<event_loop_thread_pool> thread_pool() { return thread_pool_; }

    /// Starts the server if it's not listening.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    /// Set connection callback.
    /// Not thread safe.
    void set_connection_callback(const connection_callback& cb) { connection_callback_ = cb; }

    /// Set message callback.
    /// Not thread safe.
    void set_message_callback(const message_callback& cb) { message_callback_ = cb; }

    /// Set write complete callback.
    /// Not thread safe.
    void set_write_complete_callback(const write_complete_callback& cb) { write_complete_callback_ = cb; }

private:
    /// Not thread safe, but in loop
    void new_connection(int sockfd, const inet_address& peer_addr);
    /// Thread safe.
    void remove_connection(const tcp_connection_ptr& conn);
    /// Not thread safe, but in loop
    void remove_connection_in_loop(const tcp_connection_ptr& conn);
};

}  /* namespace speed::mq  */
