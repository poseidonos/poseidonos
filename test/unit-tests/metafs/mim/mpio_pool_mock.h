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
};

} // namespace pos
