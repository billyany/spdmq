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

#include <sstream>
#include <cstring>
#include <stdexcept>

namespace speed::mq {

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#if defined __GNUC__
#define likely(x) __builtin_expect ((x), 1)
#define unlikely(x) __builtin_expect ((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

//  Provides convenient way to check for errno-style errors.
#define ERRNO_ASSERT(x)                         \
    do {                                        \
        if (unlikely (!(x))) {                  \
            std::stringstream ss;               \
            ss << std::strerror(errno)          \
            << " (" << __FILENAME__             \
            << ":" << __LINE__ << ")";          \
            throw std::runtime_error(ss.str()); \
        }                                       \
    } while (false)

#define SOCKET_ASSERT(x, y)                     \
    do {                                        \
        if (unlikely (!(x))) {                  \
            std::stringstream ss;               \
            ss << std::strerror(errno)          \
            << " (" << __FILENAME__             \
            << ":" << __LINE__ << ")";          \
            close(y);                           \
            y = -1;                             \
            throw std::runtime_error(ss.str()); \
        }                                       \
    } while (false)

} /* namespace speed::mq */