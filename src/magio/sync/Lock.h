#pragma once

#include <mutex>

namespace magio {

class SpinLock {
public:
    SpinLock() = default;
    SpinLock(const SpinLock&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;

    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire));
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag_;
};


}