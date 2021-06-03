#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mpio_pool.h"

namespace pos
{
class MockMpioPool : public MpioPool
{
public:
    using MpioPool::MpioPool;
    MOCK_METHOD(size_t, GetPoolSize, (), (override));
    MOCK_METHOD(void, Release, (Mpio* mpio), (override));
    MOCK_METHOD(void, ReleaseCache, (), (override));
};

} // namespace pos
