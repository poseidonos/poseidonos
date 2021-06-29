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
    MOCK_METHOD(bool, CanFlushMeta, (int core, FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(void, FinishMetaFlush, (), (override));
};

} // namespace pos
