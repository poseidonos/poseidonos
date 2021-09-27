#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/frontend_io/flush_command_manager.h"

namespace pos
{
class MockFlushCmdManager : public FlushCmdManager
{
public:
    using FlushCmdManager::FlushCmdManager;
    MOCK_METHOD(bool, IsFlushEnabled, (), (override));
    MOCK_METHOD(bool, CanFlushMeta, (FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(void, FinishMetaFlush, (), (override));
    MOCK_METHOD(void, UpdateVSANewEntries, (uint32_t volId, int arrayId), (override));
    MOCK_METHOD(bool, IsInternalFlushEnabled, (), (override));
    MOCK_METHOD(int, GetInternalFlushThreshold, (), (override));
    MOCK_METHOD(bool, TrySetFlushInProgress, (uint32_t volId), (override));
    MOCK_METHOD(void, ResetFlushInProgress, (uint32_t volId, bool isBackendFlush), (override));
};

} // namespace pos
