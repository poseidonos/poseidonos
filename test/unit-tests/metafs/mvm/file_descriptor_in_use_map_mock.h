#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/file_descriptor_in_use_map.h"

namespace pos
{
class MockFileDescriptorInUseMap : public FileDescriptorInUseMap
{
public:
    using FileDescriptorInUseMap::FileDescriptorInUseMap;
};

} // namespace pos
