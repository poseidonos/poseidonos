#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mvm/file_descriptor_manager.h"

namespace pos
{
class MockFileDescriptorManager : public FileDescriptorManager
{
public:
    using FileDescriptorManager::FileDescriptorManager;
};

} // namespace pos
