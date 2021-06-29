#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mdpage_buf_pool.h"

namespace pos
{
class MockMDPageBufPool : public MDPageBufPool
{
public:
    using MDPageBufPool::MDPageBufPool;
};

} // namespace pos
