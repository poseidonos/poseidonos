#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/util/metafs_spinlock.h"

namespace pos
{
class MockMetaFsSpinLock : public MetaFsSpinLock
{
public:
    using MetaFsSpinLock::MetaFsSpinLock;
};

} // namespace pos
