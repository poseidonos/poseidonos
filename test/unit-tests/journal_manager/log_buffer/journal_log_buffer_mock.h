#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/journal_log_buffer.h"

namespace pos
{
class MockJournalLogBuffer : public JournalLogBuffer
{
public:
    using JournalLogBuffer::JournalLogBuffer;
    MOCK_METHOD(int, Init, (JournalConfiguration * journalConfiguration), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(bool, IsLoaded, (), (override));
    MOCK_METHOD(int, WriteLog, (LogWriteContext * context, int logGroupID, uint64_t offset), (override));
    MOCK_METHOD(int, SyncResetAll, (), (override));
};

} // namespace pos
