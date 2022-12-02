#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/log_buffer_io_context_factory.h"

namespace pos
{
class MockLogBufferIoContextFactory : public LogBufferIoContextFactory
{
public:
    using LogBufferIoContextFactory::LogBufferIoContextFactory;
    MOCK_METHOD(void, Init, (JournalConfiguration * config, LogBufferWriteDoneNotifier* target, CallbackSequenceController* sequencer), (override));
    MOCK_METHOD(LogBufferIoContext*, CreateLogBufferIoContext, (int groupId, EventSmartPtr event), (override));
    MOCK_METHOD(MapUpdateLogWriteContext*, CreateMapUpdateLogWriteIoContext, (LogWriteContext * context), (override));
    MOCK_METHOD(LogWriteIoContext*, CreateLogWriteIoContext, (LogWriteContext * context), (override));
    MOCK_METHOD(LogBufferIoContext*, CreateLogGroupFooterWriteContext, (uint64_t offset, LogGroupFooter footer, int logGroupId, EventSmartPtr callback), (override));
};

} // namespace pos
