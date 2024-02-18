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

#include <stdint.h>
#include <endian.h>

namespace speed::mq {

// 内联汇编代码使得类型模糊，
// 所以我们暂时禁用警告。

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint16_t host2network16(uint16_t host16) {
    return htobe16(host16);
}

inline uint16_t network2host16(uint16_t net16) {
    return be16toh(net16);
}

inline uint32_t host2network32(uint32_t host32) {
    return htobe32(host32);
}

inline uint32_t network2host32(uint32_t net32) {
    return be32toh(net32);
}

inline uint64_t host2network64(uint64_t host64) {
    return htobe64(host64);
}

inline uint64_t network2host64(uint64_t net64) {
    return be64toh(net64);
}

#pragma GCC diagnostic pop

}  /* speed::mq */
