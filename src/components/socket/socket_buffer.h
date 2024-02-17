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
#include <sys/uio.h>
#include <sys/socket.h>

namespace speed::mq {

class socket_buffer {
private:
    static const char kCRLF[];
    static const std::size_t k_cheap_prepend = 8;
    static const std::size_t k_initial_size = 1024;

    std::vector<char> buffer_;
    std::size_t reader_index_;
    std::size_t writer_index_;

public:
    ssize_t recv_bytes(int32_t fd, int32_t& saved_errno);

    std::size_t readable_bytes() const;
    std::size_t writable_bytes() const;
    std::size_t prependable_bytes() const;

    char* begin_write();
    const char* begin_write() const;

    void append(const char* data, std::size_t len);
    void ensure_writable_bytes(size_t len);
    void make_space(std::size_t len);
    void has_written(std::size_t len);

private:
    char* begin();
    const char* begin() const;

}; /* class socket_buffer */

} /* namespace speed::mq */
