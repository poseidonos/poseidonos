#include "log_buffer_integration_test.h"

#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"

#include "src/journal_manager/log/log_buffer_parser.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"

#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/journal_configuration_builder.h"

#include <string>

#define MIN(a, b) ((a) > (b) ? (b) : (a));

namespace pos
{
JournalLogBufferIntegrationTest::JournalLogBufferIntegrationTest(void)
: config(nullptr),
  logBuffer(nullptr)
{
}

JournalLogBufferIntegrationTest::~JournalLogBufferIntegrationTest(void)
{
}

void
JournalLogBufferIntegrationTest::SetUp(void)
{
    numLogsWritten = 0;
    addedLogs.clear();

    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true);

    config = builder.Build();
    config->Init();

    logBuffer = new JournalLogBuffer(new MockFileIntf(GetLogFileName(), "POSArray"));
    logBuffer->Delete();
    logBuffer->Init(config);
    logBuffer->SyncResetAll();
}

void
JournalLogBufferIntegrationTest::TearDown(void)
{
    for (auto it : addedLogs)
    {
        delete it;
    }
    addedLogs.clear();

    logBuffer->Dispose();
    delete logBuffer;
    delete config;
}

void
JournalLogBufferIntegrationTest::SimulateSPOR(void)
{
    delete logBuffer;
    logBuffer = new JournalLogBuffer(new MockFileIntf(GetLogFileName(), "POSArray"));
    logBuffer->Init(config);
}

LogWriteContext*
JournalLogBufferIntegrationTest::_CreateBlockLogWriteContext(void)
{
    LogWriteContextFactory factory;
    factory.Init(new LogBufferWriteDoneNotifier());

    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 0, ""));
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(testInfo.defaultTestVol);

    MpageList dirty;

    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateBlockMapLogWriteContext(volumeIo, dirty, callback);
    context->callback = std::bind(&JournalLogBufferIntegrationTest::WriteDone, this,
        std::placeholders::_1);
    return context;
}

int
JournalLogBufferIntegrationTest::_ParseLogBuffer(int groupId, LogList& groupLogs)
{
    void* buffer = malloc(config->GetLogGroupSize());

    EXPECT_TRUE(logBuffer->ReadLogBuffer(0, buffer) == 0);

    LogBufferParser parser;
    int result = parser.GetLogs(buffer, config->GetLogGroupSize(), groupLogs);

    free(buffer);
    return result;
}

void
JournalLogBufferIntegrationTest::WriteDone(AsyncMetaFileIoCtx* ctx)
{
    numLogsWritten++;
}

void
JournalLogBufferIntegrationTest::_WaitForLogWriteDone(int numLogsWaitingFor)
{
    while (numLogsWritten != numLogsWaitingFor)
    {
    }
}

// TODO (huijeong.kim) check if it can be merged with log write tester
void
JournalLogBufferIntegrationTest::_CompareWithAdded(LogList& logs)
{
    EXPECT_TRUE(logs.size() == addedLogs.size());

    auto readIter = logs.begin();
    auto addedIter = addedLogs.begin();

    while (readIter != logs.end() && addedIter != addedLogs.end())
    {
        EXPECT_TRUE((*readIter)->GetType() == (*addedIter)->GetType());
        EXPECT_TRUE(memcmp((*readIter)->GetData(),
                        (*addedIter)->GetData(), (*readIter)->GetSize()) == 0);

        readIter++;
        addedIter++;
    }

    EXPECT_TRUE(readIter == logs.end());
    EXPECT_TRUE(addedIter == addedLogs.end());
}

TEST_F(JournalLogBufferIntegrationTest, ParseLogBuffer)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::ReadLogBufferAfterPOR");

    uint64_t offset = 0;
    uint32_t numLogsAdded = 0;

    uint64_t sizeToWrite = MIN(16 * 1024, config->GetLogBufferSize());
    while (offset < sizeToWrite)
    {
        LogWriteContext* context = _CreateBlockLogWriteContext();
        EXPECT_TRUE(logBuffer->WriteLog(context, 0, offset) == 0);
        offset += context->GetLogSize();
        numLogsAdded++;

        BlockWriteDoneLog* logData =
            reinterpret_cast<BlockWriteDoneLog*>(context->GetLog()->GetData());
        LogHandlerInterface* logHandler = new BlockWriteDoneLogHandler(*logData);
        addedLogs.push_back(logHandler);
    }

    _WaitForLogWriteDone(numLogsAdded);

    SimulateSPOR();

    LogList logs;
    EXPECT_TRUE(_ParseLogBuffer(0, logs) == 0);
    _CompareWithAdded(logs);
}

TEST_F(JournalLogBufferIntegrationTest, WriteInvalidLogType)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::WriteInvalidLogType");

    LogWriteContext* context = _CreateBlockLogWriteContext();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());

    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* typePtr = reinterpret_cast<int*>(&(log->type));
        *typePtr = -1;

        EXPECT_TRUE(logBuffer->WriteLog(context, 0, 0) == 0);
        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) != 0);
        EXPECT_TRUE(groupLogs.size() == 0);
    }
    else
    {
        delete context;
        FAIL();
    }

    delete context;
}

TEST_F(JournalLogBufferIntegrationTest, WriteWithoutMark)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::WriteWithoutMark");

    LogWriteContext* context = _CreateBlockLogWriteContext();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());

    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* markPtr = reinterpret_cast<int*>(&(log->mark));
        *markPtr = 0;

        EXPECT_TRUE(logBuffer->WriteLog(context, 0, 0) == 0);

        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) == 0);
        EXPECT_TRUE(groupLogs.size() == 0);
    }
    else
    {
        delete context;
        FAIL();
    }

    delete context;
}
} // namespace pos
