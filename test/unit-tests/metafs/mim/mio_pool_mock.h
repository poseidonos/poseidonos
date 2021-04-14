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
};

} // namespace pos
