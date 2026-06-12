#pragma once

#include <atomic>
#include <cstdint>

#include "as_sensing/stick_sample.h"

namespace as {

// Single-writer latest-value seqlock. The 1 kHz producer never blocks on a
// consumer; readers retry on a torn read. Same pattern as ATOS's clamped
// setpoint mailbox.
template <typename T>
class Mailbox {
public:
    void write(const T &v)
    {
        const uint32_t s = seq_.load(std::memory_order_relaxed);
        seq_.store(s + 1, std::memory_order_release);  // odd: write in progress
        value_ = v;
        seq_.store(s + 2, std::memory_order_release);
    }

    // Returns false until the first write, or if 4 retries all caught a
    // concurrent write (practically impossible at these rates).
    bool read(T &out) const
    {
        for (int i = 0; i < 4; ++i) {
            const uint32_t s1 = seq_.load(std::memory_order_acquire);
            if (s1 == 0 || (s1 & 1u)) {
                continue;
            }
            T tmp = value_;
            std::atomic_thread_fence(std::memory_order_acquire);
            const uint32_t s2 = seq_.load(std::memory_order_relaxed);
            if (s1 == s2) {
                out = tmp;
                return true;
            }
        }
        return false;
    }

private:
    std::atomic<uint32_t> seq_{0};
    T value_{};
};

}  // namespace as
