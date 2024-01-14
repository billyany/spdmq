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

#include "socket.h"
#include "spdmq_error.hpp"

#include <cstdint>
#include <netinet/tcp.h>

namespace speed::mq {

int32_t socket::set_nosigpipe (spdmq_fd_t fd) {
    //  Make sure that SIGPIPE signal is not generated when writing to a
    //  connection that was already closed by the peer.
    //  As per POSIX spec, EINVAL will be returned if the socket was valid but
    //  the connection has been reset by the peer. Return an error so that the
    //  socket can be closed and the connection retried if necessary.
    int32_t set = 1;
    int32_t rc = setsockopt (fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof set);
    if (rc != 0 && errno == EINVAL) {
        return -1;
    }
    return 0;
}

spdmq_fd_t socket::open_socket (int32_t domain, int32_t type, int32_t protocol) {
    spdmq_fd_t socket_fd = socket (domain, type | SOCK_CLOEXEC, protocol);
    ERRNO_ASSERT(socket_fd >= 3);

    //  Ensure that the socket is closed after the exec call
    const int rc = fcntl (socket_fd, F_SETFD, FD_CLOEXEC);
    ERRNO_ASSERT (rc != -1);

    //  Socket is not yet connected so EINVAL is not a valid networking error
    rc = zmq::set_nosigpipe (socket_fd);
    ERRNO_ASSERT (rc == 0);

    return s;
}

}
