#pragma once

#include <queue>
#include "DBusSpinLock.hpp"

/**
 * @brief 这是一个使用了 atomic 锁，线程安全的队列
 *
 */

namespace opendbus {

template<typename T>
class DBusQueue {
private:
    std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
    std::queue<T> queue_;

public:
    void front(T& value) {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        value = queue_.front();
    }

    T& front() {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        return queue_.front();
    }

    void push(const T& data) {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        queue_.emplace(data);
    }

    void push(T&& data) {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        queue_.emplace(std::move(data));
    }

    T pop() {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        T value;
        if (!queue_.empty()) {
            value = queue_.front();
            queue_.pop();
        }
        return value;
    }

    bool empty() {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        return queue_.empty();
    }

    std::size_t size() {
        DBusSpinLock<std::atomic_flag> lk(lock_);
        return queue_.size();
    }

public:
    DBusQueue() {}
    ~DBusQueue() {}
};

} // namespace hicore
