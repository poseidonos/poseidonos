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
    MOCK_METHOD(int, GetLogGroupId, (), (override));
    MOCK_METHOD(void, SetBufferAllocated, (uint64_t offset, int groupId, uint32_t seqNum), (override));
    MOCK_METHOD(void, IoDone, (), (override));
    MOCK_METHOD(int, GetError, (), (const, override));
    MOCK_METHOD(void, SetFile, (int fileDescriptor), (override));
};

} // namespace pos
