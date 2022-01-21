#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mio_pool.h"

namespace pos
{
class MockMioPool : public MioPool
{
public:
    using MioPool::MioPool;
    MOCK_METHOD(Mio*, TryAlloc, (), (override));
    MOCK_METHOD(void, Release, (Mio* mio), (override));
    MOCK_METHOD(bool, IsEmpty, (), (override));
};

} // namespace pos
