#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/lib/mfs_lockless_q.h"

namespace pos
{
template<typename Entity>
class MockMetaFsLockLessQ : public MetaFsLockLessQ<Entity>
{
public:
    using MetaFsLockLessQ::MetaFsLockLessQ;
};

} // namespace pos
