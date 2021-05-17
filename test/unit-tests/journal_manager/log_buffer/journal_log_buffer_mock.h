#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/journal_log_buffer.h"

namespace pos
{
class MockJournalLogBuffer : public JournalLogBuffer
{
public:
    using JournalLogBuffer::JournalLogBuffer;
    MOCK_METHOD(int, Init, (JournalConfiguration* journalConfiguration), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(bool, IsLoaded, (), (override));
    MOCK_METHOD(int, WriteLog, (LogWriteContext* context), (override));
    MOCK_METHOD(int, SyncResetAll, (), (override));
    MOCK_METHOD(int, AsyncReset, (int id, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(void, LogGroupResetCompleted, (int logGroupId), (override));
};

} // namespace pos
