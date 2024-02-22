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

#include <cmath>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace speed::mq {

using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
using time_stamp_ms = std::chrono::milliseconds;

class time_stamp {
public:
    static time_stamp create_invalid_time_point() {
        return std::chrono::time_point<std::chrono::steady_clock>{};
    }

    time_stamp() : time_point_(std::chrono::steady_clock::now()) {
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()).count() << std::endl;
    }

    time_stamp(time_point_t time_point) : time_point_(time_point) {}

    int64_t seconds_count() {
        return std::chrono::duration_cast<std::chrono::seconds>
               (time_point_.time_since_epoch()).count(); 
    }

    int64_t micros_count() {
        return std::chrono::duration_cast<std::chrono::microseconds>
               (time_point_.time_since_epoch()).count(); 
    }
    
    bool valid() {
        return time_point_.time_since_epoch().count() > 0;
    }

    template <typename duration_time>
    time_stamp operator+(const duration_time& duration) const {
        return time_stamp(time_point_ + duration);
    }

    template<typename T = std::chrono::milliseconds>
    std::string fotmat_string(int8_t place = 3) {
        auto duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(time_point_.time_since_epoch());
        auto system_time = std::chrono::time_point<std::chrono::system_clock>(duration);

        auto timet_t = std::chrono::system_clock::to_time_t(system_time);
        auto system_time_ms = std::chrono::duration_cast<T>(system_time.time_since_epoch()) % int64_t(std::pow(10, place));

        auto tm = *std::localtime(&timet_t);
        std::ostringstream stream;
        stream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        stream << '.' << std::setfill('0') << std::setw(place) << system_time_ms.count();

        return stream.str();
    }

private:
    time_point_t time_point_;
};

} /* namespace speed::mq */
