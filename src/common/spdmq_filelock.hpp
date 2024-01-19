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

#include <unistd.h>
#include <sys/file.h>

#include <string>
#include <sstream>
#include <stdexcept>

/**
* @brief This is a file lock with RAII functionality. The constructor has two parameters: 
*        the first parameter is the file path, and the second parameter is whether to use a write lock,
*        True - write lock, false - read lock. This lock can provide the function of one write and multiple reads, 
*        and multiple locks can be read simultaneously, but only one can be read at the same time
*        Write a lock and use it as follows:
*
*               spdmq_filelock file_lock ("example.lock", true) // Using write lock mode
*
*               spdmq_filelock file_lock ("example.lock") // Use read lock mode (default to read lock, so false can be omitted)
*/

namespace speed::mq {

class spdmq_filelock {
private:
    int fd_;
    std::string file_path_;

public:
    spdmq_filelock(const std::string& file_path, bool writable = false) : file_path_ (file_path) {
        fd_ = open(file_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd_ == -1) {
            std::stringstream ss;
            ss << "Failed to open file: " << file_path;
            throw std::runtime_error(ss.str());
        }

        int lock_type = writable ? (LOCK_EX | LOCK_NB) : (LOCK_SH | LOCK_NB);
        if (flock(fd_, lock_type) == -1) {
            std::stringstream ss;
            ss << "Failed to lock file: " << file_path;
            throw std::runtime_error(ss.str());
        }
    }

    ~spdmq_filelock() {
        unlock();
        close(fd_);
    }

    void unlock() {
        if (flock(fd_, LOCK_UN) == -1) {
            std::stringstream ss;
            ss << "Failed to unlock file: " << file_path_;
            throw std::runtime_error(ss.str());
        }
    }
};

} /* namespace speed::mq */
