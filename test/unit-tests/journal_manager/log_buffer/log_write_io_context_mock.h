#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/log_write_io_context.h"

namespace pos
{
class MockLogWriteIoContext : public LogWriteIoContext
{
public:
    using LogWriteIoContext::LogWriteIoContext;
    MOCK_METHOD(void, IoDone, (), (override));
    MOCK_METHOD(LogHandlerInterface*, GetLog, (), (override));
    MOCK_METHOD(LogWriteContext*, GetLogWriteContext, (), (override));
    MOCK_METHOD(int, GetLogGroupId, (), (override));
    MOCK_METHOD(int, GetError, (), (const, override));
};

} // namespace pos
