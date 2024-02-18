#include "event_loop.h"
#include "base_socket.h"
#include "weak_callback.h"
#include "socket_connect.h"
#include "socket_channel.h"
#include "spdmq_logger.hpp"

#include <functional>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

using namespace std::placeholders;

namespace speed::mq {

socket_connect::socket_connect(
    event_loop* loop, 
    const std::string& name,
    int sockfd,
    const inet_address& local_addr,
    const inet_address& peer_addr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(true),
      socket_(new base_socket(sockfd)),
      channel_(new socket_channel(*loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      high_water_mark_(64 * 1024 * 1024) {

    channel_->set_read_callback(std::bind(&socket_connect::handle_read, this));
    channel_->set_write_callback(std::bind(&socket_connect::handle_write, this));
    channel_->set_close_callback(std::bind(&socket_connect::handle_close, this));
    channel_->set_error_callback(std::bind(&socket_connect::handle_error, this));
    socket_->set_keep_alive(true);
    LOG_DEBUG("socket_connect::socket_connect [%s] at %p, fd=%d", name_.data(), this, sockfd);
}

socket_connect::~socket_connect() {
    LOG_DEBUG("socket_connect::~socket_connect [%s] at %p fd=%d state=%s", 
               name_.data(), this, channel_->fd(), state2string());
    assert(state_ == kDisconnected);
}

event_loop* socket_connect::get_loop() const {
    return loop_;
}

const std::string& socket_connect::name() const { 
    return name_;
}
const inet_address& socket_connect::local_address() const {
    return local_addr_;
}

const inet_address& socket_connect::peer_address() const {
    return peer_addr_;
}

bool socket_connect::connected() const {
    return state_ == kConnected;
}

bool socket_connect::disconnected() const { 
    return state_ == kDisconnected;
}

bool socket_connect::get_tcp_info(struct tcp_info* tcpi) const {
    return socket_->get_tcp_info(tcpi);
}

std::string socket_connect::get_tcp_info_string() const {
    char buf[1024] = {};
    socket_->get_tcp_info_string(buf, sizeof buf);
    return buf;
}

void socket_connect::send(const void* data, int len) {
    send(std::string(static_cast<const char*>(data), len));
}

void socket_connect::send(const std::string& message) {
    if (state_ != kConnected) return;
    if (loop_->is_in_loop_thread()) {
        send_in_loop(message);
    } else {
        void (socket_connect::*fp)(const std::string& message) = &socket_connect::send_in_loop;
        loop_->run_in_loop(std::bind(fp, this, message));
    }
}

void socket_connect::send(socket_buffer* buf) {
    if (state_ != kConnected) return;
    if (loop_->is_in_loop_thread()) {
        send_in_loop(buf->peek(), buf->readable_bytes());
        buf->retrieve_all();
    } else {
        void (socket_connect::*fp)(const std::string& message) = &socket_connect::send_in_loop;
        loop_->run_in_loop(std::bind(fp, this, buf->retrieve_all_as_string()));
    }
}

void socket_connect::send_in_loop(const std::string& message) {
    send_in_loop(message.data(), message.size());
}

void socket_connect::send_in_loop(const void* data, size_t len) {
    loop_->assert_in_loop_thread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state_ == kDisconnected) {
        LOG_WARN("disconnected, give up writing");
        return;
    }

    // if no thing in output queue, try writing directly
    if (!channel_->is_writing() && output_buffer_.readable_bytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && write_complete_callback_) {
                loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
            }
        } else { // nwrote < 0
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                // LOG_SYSERR << "socket_connect::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) { // FIXME: any others?
                    fault_error = true;
                }
            }
        }
    }

    assert(remaining <= len);

    if (!fault_error && remaining > 0) {
        size_t old_len = output_buffer_.readable_bytes();
        if (old_len + remaining >= high_water_mark_ && old_len < high_water_mark_ && high_water_mark_callback_) {
            loop_->queue_in_loop(std::bind(high_water_mark_callback_, shared_from_this(), old_len + remaining));
        }
        output_buffer_.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!channel_->is_writing()) {
            channel_->enable_writing();
        }
    }
}

void socket_connect::shutdown() {
    if (state_ == kConnected) {
        set_state(kDisconnecting);
        loop_->run_in_loop(std::bind(&socket_connect::shutdown_in_loop, shared_from_this()));
    }
}

void socket_connect::shutdown_in_loop() {
    loop_->assert_in_loop_thread();
    if (!channel_->is_writing()) {
        socket_->shutdown_write();
    }
}

void socket_connect::force_close() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        set_state(kDisconnecting);
        loop_->queue_in_loop(std::bind(&socket_connect::force_close_in_loop, shared_from_this()));
    }
}

void socket_connect::force_close_with_delay(double seconds)
{
    if (state_ == kConnected || state_ == kDisconnecting) {
        set_state(kDisconnecting);
        // not forceCloseInLoop to avoid race condition
        // TODO 
        // loop_->runAfter(seconds, weak_bind(shared_from_this(), &socket_connect::force_close));
    }
}

void socket_connect::force_close_in_loop() {
    loop_->assert_in_loop_thread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        // as if we received 0 byte in handleRead();
        handle_close();
    }
}

const char* socket_connect::state2string() const {
    switch (state_) {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void socket_connect::set_tcp_no_delay(bool on) {
    // TODO
    socket_->set_tcp_no_delay(on);
}

void socket_connect::start_read() {
    loop_->run_in_loop(std::bind(&socket_connect::start_read_in_loop, this));
}

void socket_connect::start_read_in_loop() {
    loop_->assert_in_loop_thread();
    if (!reading_ || !channel_->is_reading()) {
        channel_->enable_reading();
        reading_ = true;
    }
}

void socket_connect::stop_read() {
    loop_->run_in_loop(std::bind(&socket_connect::stop_read_in_loop, this));
}

void socket_connect::stop_read_in_loop() {
    loop_->assert_in_loop_thread();
    if (reading_ || channel_->is_reading()) {
        channel_->disable_reading();
        reading_ = false;
    }
}

void socket_connect::connect_established() {
    loop_->assert_in_loop_thread();
    assert(state_ == kConnecting);
    set_state(kConnected);
    channel_->tie(shared_from_this());
    channel_->enable_reading();
    connection_callback_(shared_from_this());
}

void socket_connect::connect_destroyed()
{
    loop_->assert_in_loop_thread();
    if (state_ == kConnected) {
        set_state(kDisconnected);
        channel_->disable_all();
        connection_callback_(shared_from_this());
    }
    channel_->remove();
}

void socket_connect::handle_read() {
    loop_->assert_in_loop_thread();
    int saved_errno = 0;
    ssize_t n = input_buffer_.recv_bytes(channel_->fd(), saved_errno);
    if (n > 0) {
        message_callback_(shared_from_this(), &input_buffer_);
    }
    else if (n == 0) {
        handle_close();
    }
    else {
        errno = saved_errno;
        handle_error();
        // LOG_SYSERR << "socket_connect::handleRead";
        
    }
}

void socket_connect::handle_write() {
    loop_->assert_in_loop_thread();
    if (channel_->is_writing()) {
        ssize_t n = ::write(channel_->fd(), output_buffer_.peek(), output_buffer_.readable_bytes());
        if (n > 0)
        {
            output_buffer_.retrieve(n);
            if (output_buffer_.readable_bytes() == 0) {
                channel_->disable_writing();
                if (write_complete_callback_) {
                    loop_->queue_in_loop(std::bind(write_complete_callback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdown_in_loop();
                }
            }
        } else {
            // TODO
            // LOG_SYSERR << "socket_connect::handleWrite";

            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    } else {
        // TODO
        // LOG_TRACE << "Connection fd = " << channel_->fd() << " is down, no more writing";
    }
}

void socket_connect::handle_close() {
    loop_->assert_in_loop_thread();
    // LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    set_state(kDisconnected);
    channel_->disable_all();

    tcp_connection_ptr guard_this(shared_from_this());
    connection_callback_(guard_this);
    // must be the last line
    close_callback_(guard_this);
}

void socket_connect::handle_error() {
    // TODO
    // int err = sockets::getSocketError(channel_->fd());
    // LOG_ERROR << "socket_connect::handleError [" << name_
    // << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

} /* namespace speed::mq */