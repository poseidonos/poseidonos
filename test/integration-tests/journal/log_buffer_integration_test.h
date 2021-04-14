#pragma once

#include <atomic>
#include <list>

#include "gtest/gtest.h"
#include "src/event_scheduler/event.h"

#include "src/journal_manager/log_buffer/journal_write_context.h"
#include "src/journal_manager/log_buffer/journal_log_buffer.h"

#include "test/integration-tests/journal/journal_configuration_spy.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
class LogBufferWriteDone : public Event
{
public:
    LogBufferWriteDone(void)
    {
    }

    bool
    Execute(void)
    {
        return true;
    }
};

class JournalLogBufferIntegrationTest : public ::testing::Test
{
public:
    JournalLogBufferIntegrationTest(void);
    virtual ~JournalLogBufferIntegrationTest(void);

    void WriteDone(AsyncMetaFileIoCtx* ctx);
    void SimulateSPOR(void);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    LogWriteContext* _CreateBlockLogWriteContext(void);
    int _ParseLogBuffer(int groupId, LogList& logs);
    void _WaitForLogWriteDone(int numLogsWaitingFor);
    void _CompareWithAdded(LogList& logs);

    JournalConfiguration* config;
    JournalLogBuffer* logBuffer;

    std::list<LogHandlerInterface*> addedLogs;

private:
    TestInfo testInfo;
    std::atomic<int> numLogsWritten;
};
} // namespace pos
