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
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Close, (), (override));
    MOCK_METHOD(int, LoadSync, (int owner, char* buf), (override));
    MOCK_METHOD(int, StoreSync, (int owner, char* buf), (override));
    MOCK_METHOD(int, StoreAsync, (int owner, char* buf, MetaIoCbPtr callback), (override));
};

} // namespace pos
