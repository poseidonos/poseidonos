#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/common/instance_tagid_allocator.h"

namespace pos
{
class MockInstanceTagIdAllocator : public InstanceTagIdAllocator
{
public:
    using InstanceTagIdAllocator::InstanceTagIdAllocator;
};

} // namespace pos
