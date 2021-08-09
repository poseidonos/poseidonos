#pragma once
#include <atomic>

// to reduce dependency with lib folder. just use raw api.
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

namespace pos
{
template<typename T>
class AtomicCount
{
public:
    explicit AtomicCount(T t)
    : pendingCount(t)
    {
    }
    // postfix overloading
    void operator++(int)
    {
        // We do not check overflow, If it is needed, please override.
        pendingCount++;
    }
    void operator--(int)
    {
        if (unlikely(pendingCount == 0))
        {
            ErrorLogUnderflow();
        }
        pendingCount--;
    }
    virtual void ErrorLogUnderflow(void)
    {
        return;
    }

private:
    std::atomic<T> pendingCount;
};
} // namespace pos
