#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/context_io_manager.h"

namespace pos
{
class MockContextIoManager : public ContextIoManager
{
public:
    using ContextIoManager::ContextIoManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, char* externalBuf), (override));
    MOCK_METHOD(void, WaitPendingIo, (IOTYPE type), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(char*, GetContextSectionAddr, (int owner, int section), (override));
    MOCK_METHOD(int, GetContextSectionSize, (int owner, int section), (override));
};

} // namespace pos
