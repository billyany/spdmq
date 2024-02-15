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

#include <regex>
#include <chrono>
#include <thread>
#include <cstring>

namespace speed::mq {

inline void mem_zero(void* p, size_t n) {
    memset(p, 0, n);
}

inline bool regex_match(const std::string& url, const std::string& re) {
    std::regex regex(re);
    if (std::regex_match(url, regex)) {
        return true;
    }
    return false;
}

inline int64_t now_usecs_timestamp() {
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Convert to microseconds since epoch (January 1, 1970)
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    return microseconds;
}

inline void sleep_s(uint64_t s) {
    std::this_thread::sleep_for(std::chrono::seconds(s));
}

inline void sleep_ms(uint64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void sleep_us(uint64_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

} /* namespace speed::mq */
