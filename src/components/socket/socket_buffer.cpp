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
#include <assert.h>
#include "socket_buffer.h"

namespace speed::mq {

ssize_t socket_buffer::recv_bytes(int32_t fd, int32_t& saved_errno) {

    char extra_buffer[65536];
    struct iovec vec[2];
    const std::size_t writable = writable_bytes();
    vec[0].iov_base = begin() + writer_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof extra_buffer;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < sizeof extra_buffer) ? 2 : 1;
    const ssize_t recv_size = readv(fd, vec, iovcnt);
    if (recv_size < 0) {
        saved_errno = errno;
    } else if (static_cast<std::size_t>(recv_size) <= writable) {
        writer_index_ += recv_size;
    } else {
        writer_index_ = buffer_.size();
        append(extra_buffer, recv_size - writable);
    }
    return recv_size;
}

const char* socket_buffer::peek() const { 
    return begin() + reader_index_;
}

std::size_t socket_buffer::readable_bytes() const {
    return writer_index_ - reader_index_;
}

std::size_t socket_buffer::writable_bytes() const {
    return buffer_.size() - writer_index_;
}

std::size_t socket_buffer::prependable_bytes() const {
    return reader_index_;
}

void socket_buffer::retrieve_all() {
    reader_index_ = k_cheap_prepend;
    writer_index_ = k_cheap_prepend;
}

void socket_buffer::retrieve(size_t len) {
    assert(len <= readable_bytes());
    if (len < readable_bytes()) {
        reader_index_ += len;
    }
    else {
        retrieve_all();
    }
}

std::string socket_buffer::retrieve_all_as_string() {
    return retrieve_as_string(readable_bytes());
}

std::string socket_buffer::retrieve_as_string(size_t len) {
    assert(len <= readable_bytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

char* socket_buffer::begin_write() {
    return begin() + writer_index_;
}

const char* socket_buffer::begin_write() const {
    return begin() + writer_index_;
}

void socket_buffer::append(const char* data, std::size_t len) {
    ensure_writable_bytes(len);
    std::copy(data, data + len, begin_write());
    has_written(len);
}

void socket_buffer::ensure_writable_bytes(size_t len) {
    if (writable_bytes() < len) {
        make_space(len);
    }
    assert(writable_bytes() >= len);
}

void socket_buffer::make_space(std::size_t len) {
    if (writable_bytes() + prependable_bytes() < len + k_cheap_prepend) {
        buffer_.resize(writer_index_ + len);
    } else {
        // move readable data to the front, make space inside buffer
        assert(k_cheap_prepend < reader_index_);
        
        size_t readable = readable_bytes();
        std::copy(begin() + reader_index_,
        begin() + writer_index_,
        begin() + k_cheap_prepend);

        reader_index_ = k_cheap_prepend;
        writer_index_ = reader_index_ + readable;

        assert(readable == readable_bytes());
    }
}

void socket_buffer::has_written(std::size_t len) {
    assert(len <= writable_bytes());
    writer_index_ += len;
}

char* socket_buffer::begin() { 
    return &*buffer_.begin();
}

const char* socket_buffer::begin() const {
    return &*buffer_.begin();
}

} /* namespace speed::mq */