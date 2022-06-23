#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/i_journal_log_buffer.h"

namespace pos
{
class MockIJournalLogBuffer : public IJournalLogBuffer
{
public:
    using IJournalLogBuffer::IJournalLogBuffer;
    MOCK_METHOD(int, Init, (JournalConfiguration* journalConfiguration, LogWriteContextFactory* logWriteContextFactory, int arrayId, TelemetryPublisher* tp), (override));
    MOCK_METHOD(void, InitDataBuffer, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, Create, (uint64_t logBufferSize), (override));
    MOCK_METHOD(int, Open, (uint64_t& logBufferSize), (override));
    MOCK_METHOD(int, ReadLogBuffer, (int groupId, void* buffer), (override));
    MOCK_METHOD(int, WriteLog, (LogWriteContext* context), (override));
    MOCK_METHOD(int, SyncResetAll, (), (override));
    MOCK_METHOD(int, AsyncReset, (int id, EventSmartPtr callbackEvent), (override));
    MOCK_METHOD(int, InternalIo, (LogBufferIoContext* context), (override));
    MOCK_METHOD(void, InternalIoDone, (AsyncMetaFileIoCtx* ctx), (override));
    MOCK_METHOD(int, Delete, (), (override));
    MOCK_METHOD(void, LogGroupResetCompleted, (int logGroupId), (override));
    MOCK_METHOD(bool, DoesLogFileExist, (), (override));
};

} // namespace pos
