
#include "src/lib/Lock.h"

class LockTest : public ::testing::Test
{
public:
    SpinLock* lock{nullptr};

protected:
    LockTest()
    {
        lock = new SpinLock{};
    }
    ~LockTest() override
    {
        if (nullptr != lock)
        {
            delete lock;
            lock = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
