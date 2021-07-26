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
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, UpdateSectionInfo, (int owner, int section, char* addr, int size, int offset), (override));
    MOCK_METHOD(int, LoadSync, (int owner, char* buf), (override));
    MOCK_METHOD(int, StoreSync, (int owner, char* buf), (override));
    MOCK_METHOD(int, StoreAsync, (int owner, char* buf, MetaIoCbPtr callback), (override));
    MOCK_METHOD(void, LoadSectionData, (int owner, char* buf), (override));
    MOCK_METHOD(void, CopySectionData, (int owner, char* buf, int startSection, int endSection), (override));
    MOCK_METHOD(int, GetFileSize, (int owner), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int owner, int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int owner, int section), (override));
    MOCK_METHOD(int, GetSectionOffset, (int owner, int section), (override));
};

} // namespace pos
