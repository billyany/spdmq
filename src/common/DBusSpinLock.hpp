#pragma once

#include <atomic>

namespace opendbus {

template<typename T>
class DBusSpinLock {
private:
    T* lock_;

public:
    explicit DBusSpinLock(T& lock) : lock_(std::addressof(lock))
    {
        while (lock_->test_and_set(std::memory_order_acquire));
    }

    ~DBusSpinLock()
    {
        lock_->clear(std::memory_order_release);
    }
};

} /* namespace opendbus */
