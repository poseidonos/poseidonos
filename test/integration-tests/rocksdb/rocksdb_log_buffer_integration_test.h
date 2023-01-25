#pragma once

#include <atomic>
#include <list>

#include "gtest/gtest.h"
#include "src/event_scheduler/event.h"
#include "src/journal_manager/log/log_list.h"
#include "src/journal_manager/log_buffer/log_buffer_io_context_factory.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_buffer/map_update_log_write_context.h"
#include "src/rocksdb_log_buffer/rocksdb_log_buffer.h"
#include "test/integration-tests/journal/log_buffer_integration_test.h"

namespace pos
{
using ::testing::NiceMock;

// TODO Refactoring - LogBufferIntegrationTest + RocksDBLogBufferIntegrationTest
class RocksDBLogBufferIntegrationTest : public ::testing::Test
{
public:
    RocksDBLogBufferIntegrationTest(void);
    virtual ~RocksDBLogBufferIntegrationTest(void);

    void WriteDone(AsyncMetaFileIoCtx* ctx);
    void SimulateSPOR(void);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    LogWriteContext* _CreateContextForBlockWriteDoneLog(void);
    LogWriteContext* _CreateContextForStripeMapUpdatedLog(void);
    LogWriteContext* _CreateContextForGcBlockWriteDoneLog(void);
    LogWriteContext* _CreateContextForGcStripeFlushedLog(void);

    int _ParseLogBuffer(int groupId, LogList& logs);
    void _WaitForLogWriteDone(int numLogsWaitingFor);

    void _CompareWithAdded(LogList& logs);
    bool _CheckLogInTheList(LogHandlerInterface* log);
    void _AddToList(LogWriteContext* context);

    const int NUM_LOG_GROUPS = 2;
    const uint64_t LOG_BUFFER_SIZE = 128 * 1024;
    const uint64_t LOG_GROUP_SIZE = LOG_BUFFER_SIZE / NUM_LOG_GROUPS;
    const std::string rocksdbPath = "/etc/pos/POSRaid";

    const int TEST_VOLUME_ID = 1;

    NiceMock<MockJournalConfiguration> config;

    LogWriteContextFactory factory;
    LogBufferIoContextFactory ioContextFactory;
    RocksDBLogBuffer* journalRocks;

    std::list<LogHandlerInterface*> addedLogs;

private:
    int _PrepareLogBuffer(void);

    TestInfo testInfo;
    std::atomic<int> numLogsWritten;
};
} // namespace pos
