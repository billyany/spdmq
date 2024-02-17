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
#include <any>
#include <memory>
#include <functional>
#include "spdmq_uncopyable.h"
#include "socket_buffer.h"


// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace speed::mq {

class socket_channel;
class event_loop;
// class Socket;
class socket_connect;
typedef std::shared_ptr<socket_connect> tcp_connection_ptr;
typedef std::function<void (const tcp_connection_ptr&)> connection_callback;
typedef std::function<void (const tcp_connection_ptr&)> close_callback;
typedef std::function<void (const tcp_connection_ptr&)> write_complete_callback;
typedef std::function<void (const tcp_connection_ptr&, size_t)> high_water_mark_callback;
typedef std::function<void (const tcp_connection_ptr&, socket_buffer*)> message_callback;

class socket_connect : spdmq_uncopyable, public std::enable_shared_from_this<socket_connect> {
private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    event_loop* loop_;
    const std::string name_;
    StateE state_;  // FIXME: use atomic variable
    bool reading_;
    // we don't expose those classes to client.
    //   std::unique_ptr<Socket> socket_;
    std::unique_ptr<socket_channel> channel_;
    connection_callback connection_callback_;
    message_callback message_callback_;
    write_complete_callback write_complete_callback_;
    high_water_mark_callback high_water_mark_callback_;
    close_callback close_callback_;
    size_t high_water_mark_;
    socket_buffer input_buffer_;
    socket_buffer output_buffer_; // FIXME: use list<Buffer> as output buffer.
    std::any context_;
    // FIXME: creationTime_, lastReceiveTime_
    //        bytesReceived_, bytesSent_

public:
    socket_connect(event_loop* loop,
                   const std::string& name,
                   int sockfd);
    ~socket_connect();

    event_loop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    //   const InetAddress& localAddress() const { return localAddr_; }
    //   const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    // return true if success.
    // bool get_tcp_info(struct tcp_info*) const;
    // std::string get_tcp_info_string() const;

    void send(const std::string& message);
    void send(std::string&& message); // C++11
    void send(const void* message, int len);
    void send(socket_buffer&& message); // C++11
    void send(socket_buffer* message);  // this one will swap data
    void shutdown(); // NOT thread safe, no simultaneous calling
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
    void force_close();
    void force_close_with_delay(double seconds);
    void set_tcp_no_delay(bool on);
    // reading or not
    void start_read();
    void stop_read();
    bool is_reading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop

    void set_context(const std::any& context)
    { context_ = context; }

    const std::any& getContext() const
    { return context_; }

    std::any& get_mutable_context()
    { return context_; }

    void set_connection_callback(const connection_callback& cb)
    { connection_callback_ = cb; }

    void set_message_callback(const message_callback& cb)
    { message_callback_ = cb; }

    void set_write_complete_callback(const write_complete_callback& cb)
    { write_complete_callback_ = cb; }

    void set_high_water_mark_callback(const high_water_mark_callback& cb, size_t high_water_mark) {
        high_water_mark_callback_ = cb;
        high_water_mark_ = high_water_mark;
    }

    /// Advanced interface
    socket_buffer* input_buffer() {
        return &input_buffer_;
    }

    socket_buffer* output_buffer() {
        return &output_buffer_;
    }

    /// Internal use only.
    void set_close_callback(const close_callback& cb)
    { close_callback_ = cb; }

    // called when TcpServer accepts a new connection
    void connect_established();   // should be called only once
    // called when TcpServer has removed me from its map
    void connect_destroyed();  // should be called only once

 private:
    void handle_read();
    void handle_write();
    void handle_close();
    void handle_error();
    // void send_in_loop(std::string&& message);
    void send_in_loop(const std::string& message);
    void send_in_loop(const void* message, size_t len);
    void shutdown_in_loop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void force_close_in_loop();
    void set_state(StateE s) { state_ = s; }
    const char* state_to_string() const;
    void start_read_in_loop();
    void stop_read_in_loop();
};

}  /* namespace speed::mq */
