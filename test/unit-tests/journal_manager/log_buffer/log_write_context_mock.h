#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/log_write_context.h"

namespace pos
{
class MockLogWriteContext : public LogWriteContext
{
public:
    using LogWriteContext::LogWriteContext;
    MOCK_METHOD(void, SetLogAllocated, (int logGroupId, uint64_t sequenceNumber), (override));
    MOCK_METHOD(MapList&, GetDirtyMapList, (), (override));
    MOCK_METHOD(int, GetLogGroupId, (), (override));
    MOCK_METHOD(uint64_t, GetLogSize, (), (override));
    MOCK_METHOD(char*, GetBuffer, (), (override));
    MOCK_METHOD(EventSmartPtr, GetCallback, (), (override));
    MOCK_METHOD(LogHandlerInterface*, GetLog, (), (override));
};

} // namespace pos
