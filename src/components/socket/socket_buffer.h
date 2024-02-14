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

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

#include <vector>
#include <memory_resource>
#include <sys/socket.h>
#include <sys/uio.h>
#include <assert.h>

namespace speed::mq {

static uint64_t g_buffer_size = 10 * 1024 * 1024;
static char* g_buffer = new char[g_buffer_size]();
static std::pmr::monotonic_buffer_resource g_buffer_pool(g_buffer, g_buffer_size);

class socket_buffer {
private:
    static const std::size_t k_cheap_prepend = 8;
    static const std::size_t k_initial_size = 1024;
    std::vector<char> buffer_;
    std::size_t reader_index_;
    std::size_t writer_index_;
    static const char kCRLF[];

private:
    char* begin() { 
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }

public:
    std::size_t readable_bytes() const {
        return writer_index_ - reader_index_;
    }

    std::size_t writable_bytes() const {
        return buffer_.size() - writer_index_;
    }

    std::size_t prependable_bytes() const {
        return reader_index_;
    }

    char* begin_write() {
        return begin() + writer_index_;
    }

    const char* begin_write() const {
        return begin() + writer_index_;
    }

    void has_written(std::size_t len) {
        assert(len <= writable_bytes());
        writer_index_ += len;
    }

    void make_space(std::size_t len) {
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

    void ensure_writable_bytes(size_t len) {
        if (writable_bytes() < len) {
            make_space(len);
        }
        assert(writable_bytes() >= len);
    }


    void append(const char* data, std::size_t len) {
        ensure_writable_bytes(len);
        std::copy(data, data + len, begin_write());
        has_written(len);
    }


public:
    socket_buffer() {
    }

    ssize_t recv_bytes(int32_t fd, int32_t& saved_errno) {

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

private:
    std::pmr::string payload_read_;
    std::pmr::string payload_write_;
}; /* class socket_buffer */

} /* namespace speed::mq */
