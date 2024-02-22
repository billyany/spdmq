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

// #include <cmath>
// #include <string>
#include <chrono>
#include <cstdint>
// #include <sstream>
// #include <iomanip>
// #include <iostream>

namespace speed::mq {

using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
using time_duration_ms = std::chrono::milliseconds;

class time_point {
private:
    time_point_t point_;

public:
    static time_point now() {
        return time_point(std::chrono::steady_clock::now());
    }

    static time_point create_invalid_time_point() {
        return std::chrono::time_point<std::chrono::steady_clock>{};
    }

    time_point(time_point_t time_point) : point_(time_point) {}

    int64_t seconds_count() {
        return std::chrono::duration_cast<std::chrono::seconds>
               (point_.time_since_epoch()).count(); 
    }

    int64_t micros_count() {
        return std::chrono::duration_cast<std::chrono::microseconds>
               (point_.time_since_epoch()).count(); 
    }
    
    bool valid() {
        return point_.time_since_epoch().count() > 0;
    }

    template <typename duration_time>
    time_point operator+(const duration_time& duration) const {
        return time_point(point_ + duration);
    }

    template<typename T = std::chrono::microseconds>
    int64_t operator-(const time_point& point) const {
        return std::chrono::duration_cast<T>(point_.time_since_epoch()).count() - 
        std::chrono::duration_cast<T>(point.point_.time_since_epoch()).count();
    }

    template<typename T = std::chrono::microseconds>
    bool operator < (const time_point& point) const {
        return std::chrono::duration_cast<T>(point_.time_since_epoch()).count() < 
        std::chrono::duration_cast<T>(point.point_.time_since_epoch()).count();
    }

    template<typename T = std::chrono::microseconds>
    bool operator > (const time_point& point) const {
        return std::chrono::duration_cast<T>(point_.time_since_epoch()).count() > 
        std::chrono::duration_cast<T>(point.point_.time_since_epoch()).count();
    }
};

inline time_point add_time(time_point timestamp, int64_t milliseconds) {
    return timestamp + std::chrono::milliseconds(milliseconds);
}

} /* namespace speed::mq */
