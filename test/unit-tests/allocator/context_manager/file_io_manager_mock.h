#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/file_io_manager.h"

namespace pos
{
class MockAllocatorFileIoManager : public AllocatorFileIoManager
{
public:
    using AllocatorFileIoManager::AllocatorFileIoManager;
};

} // namespace pos
