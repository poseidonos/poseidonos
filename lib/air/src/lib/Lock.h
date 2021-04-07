
#ifndef AIR_LOCK_H
#define AIR_LOCK_H

#include <atomic>

class SpinLock
{
public:
    SpinLock(void) = default;
    virtual ~SpinLock(void) = default;

    inline void
    Lock(void)
    {
        while (flag.test_and_set(std::memory_order_acquire))
        {
        }
    }
    inline bool
    TryLock(void)
    {
        return !flag.test_and_set(std::memory_order_acquire);
    }
    inline void
    Unlock(void)
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag{ATOMIC_FLAG_INIT};
};

#endif // AIR_LOCK_H
