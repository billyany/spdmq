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

namespace speed::mq {

inline bool regex_match(const std::string& url, const std::string& re) {
    std::regex regex(re);
    if (std::regex_match(url, regex)) {
        return true;
    }
    return false;
}

inline int64_t now_usecs_timestamp() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为自 epoch（1970年1月1日）以来的微秒数
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    return microseconds;
}

} /* speed::mq */
