#include "log_buffer_integration_test.h"

#include <string>

#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/log_buffer_parser.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"

namespace pos
{
using ::testing::Return;

JournalLogBufferIntegrationTest::JournalLogBufferIntegrationTest(void)
: logBuffer(nullptr)
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

    ON_CALL(config, IsEnabled).WillByDefault(Return(true));
    ON_CALL(config, GetLogBufferSize).WillByDefault(Return(LOG_BUFFER_SIZE));
    ON_CALL(config, GetLogGroupSize).WillByDefault(Return(LOG_GROUP_SIZE));

    // TODO(huijeong.kim) This injected modules should be deleted
    factory.Init(&config, new LogBufferWriteDoneNotifier(), new CallbackSequenceController());

    logBuffer = new JournalLogBuffer(new MockFileIntf(GetLogFileName(), 0));
    logBuffer->Delete();

    _PrepareLogBuffer();
    logBuffer->Init(&config, &factory, 0);
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

    logBuffer->Delete();
    logBuffer->Dispose();
    delete logBuffer;
}

void
JournalLogBufferIntegrationTest::SimulateSPOR(void)
{
    delete logBuffer;
    logBuffer = new JournalLogBuffer(new MockFileIntf(GetLogFileName(), 0));
    _PrepareLogBuffer();
    logBuffer->Init(&config, &factory, 0);
}

int
JournalLogBufferIntegrationTest::_PrepareLogBuffer(void)
{
    int result = 0;
    if (logBuffer->DoesLogFileExist() == true)
    {
        uint64_t logBufferSize = 0;
        result = logBuffer->Open(logBufferSize);
        if (result < 0)
        {
            return result;
        }

        EXPECT_CALL(config, GetLogBufferSize).WillRepeatedly(Return(logBufferSize));
        EXPECT_CALL(config, GetLogGroupSize).WillRepeatedly(Return(logBufferSize / NUM_LOG_GROUPS));
    }
    else
    {
        result = logBuffer->Create(LOG_BUFFER_SIZE);
    }
    return result;
}

LogWriteContext*
JournalLogBufferIntegrationTest::_CreateContextForBlockWriteDoneLog(void)
{
    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 0, 0));
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(TEST_VOLUME_ID);

    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateBlockMapLogWriteContext(volumeIo, callback);
    context->SetInternalCallback(std::bind(&JournalLogBufferIntegrationTest::WriteDone,
        this, std::placeholders::_1));
    return context;
}

LogWriteContext*
JournalLogBufferIntegrationTest::_CreateContextForStripeMapUpdatedLog(void)
{
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    StripeAddr oldAddr =
        {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = 0};
    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateStripeMapLogWriteContext(stripe, oldAddr, callback);
    context->SetInternalCallback(std::bind(&JournalLogBufferIntegrationTest::WriteDone,
        this, std::placeholders::_1));

    return context;
}

LogWriteContext*
JournalLogBufferIntegrationTest::_CreateContextForGcBlockWriteDoneLog(void)
{
    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = TEST_VOLUME_ID;
    mapUpdates.vsid = 100;
    mapUpdates.wbLsid = 2;
    mapUpdates.userLsid = 100;

    for (int offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate mapUpdate;
        mapUpdate.rba = offset;
        mapUpdate.vsa.stripeId = 100;
        mapUpdate.vsa.offset = offset;

        mapUpdates.blockMapUpdateList.push_back(mapUpdate);
    }

    EventSmartPtr callback(new LogBufferWriteDone());
    LogWriteContext* context =
        factory.CreateGcBlockMapLogWriteContext(mapUpdates, callback);
    context->SetInternalCallback(std::bind(&JournalLogBufferIntegrationTest::WriteDone,
        this, std::placeholders::_1));

    return context;
}

LogWriteContext*
JournalLogBufferIntegrationTest::_CreateContextForGcStripeFlushedLog(void)
{
    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = TEST_VOLUME_ID;
    mapUpdates.vsid = 100;
    mapUpdates.wbLsid = 2;
    mapUpdates.userLsid = 100;

    for (int offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate mapUpdate;
        mapUpdate.rba = offset;
        mapUpdate.vsa.stripeId = 100;
        mapUpdate.vsa.offset = offset;

        mapUpdates.blockMapUpdateList.push_back(mapUpdate);
    }

    EventSmartPtr callback(new LogBufferWriteDone());
    LogWriteContext* context =
        factory.CreateGcStripeFlushedLogWriteContext(mapUpdates, callback);
    context->SetInternalCallback(std::bind(&JournalLogBufferIntegrationTest::WriteDone,
        this, std::placeholders::_1));

    return context;
}

int
JournalLogBufferIntegrationTest::_ParseLogBuffer(int groupId, LogList& groupLogs)
{
    void* buffer = malloc(LOG_GROUP_SIZE);

    EXPECT_TRUE(logBuffer->ReadLogBuffer(groupId, buffer) == 0);

    LogBufferParser parser;
    int result = parser.GetLogs(buffer, LOG_GROUP_SIZE, groupLogs);

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
JournalLogBufferIntegrationTest::_CompareWithAdded(LogList& logList)
{
    std::list<LogHandlerInterface*> logs = logList.GetLogs();
    EXPECT_EQ(logs.size(), addedLogs.size());

    auto readIter = logs.begin();
    auto addedIter = addedLogs.begin();

    for (int count = 0; count < logs.size(); count++)
    {
        EXPECT_EQ((*readIter)->GetType(), (*addedIter)->GetType());
        EXPECT_TRUE(memcmp((*readIter)->GetData(),
                        (*addedIter)->GetData(), (*readIter)->GetSize()) == 0);

        readIter++;
        addedIter++;
    }

    EXPECT_TRUE(readIter == logs.end());
    EXPECT_TRUE(addedIter == addedLogs.end());
}

void
JournalLogBufferIntegrationTest::_AddToList(LogWriteContext* context)
{
    LogHandlerInterface* handler;

    LogType logType = context->GetLog()->GetType();
    char* data = context->GetLog()->GetData();

    if (logType == LogType::BLOCK_WRITE_DONE)
    {
        handler = new BlockWriteDoneLogHandler(*(reinterpret_cast<BlockWriteDoneLog*>(data)));
    }
    else if (logType == LogType::STRIPE_MAP_UPDATED)
    {
        handler = new StripeMapUpdatedLogHandler(*(reinterpret_cast<StripeMapUpdatedLog*>(data)));
    }
    else if (logType == LogType::GC_BLOCK_WRITE_DONE)
    {
        handler = new GcBlockWriteDoneLogHandler(data);
    }
    else if (logType == LogType::GC_STRIPE_FLUSHED)
    {
        handler = new GcStripeFlushedLogHandler(*(reinterpret_cast<GcStripeFlushedLog*>(data)));
    }
    addedLogs.push_back(handler);
}

TEST_F(JournalLogBufferIntegrationTest, ParseLogBuffer_testIfAllLogsAreParsed)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::ParseLogBuffer_testIfAllLogsAreParsed");

    uint64_t offset = 0;

    // Write block write done log
    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write stripe map updated log
    context = _CreateContextForStripeMapUpdatedLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write gc block write done log
    context = _CreateContextForGcBlockWriteDoneLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write gc stripe flushed log
    context = _CreateContextForGcStripeFlushedLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    _WaitForLogWriteDone(addedLogs.size());

    SimulateSPOR();

    LogList logs;
    EXPECT_TRUE(_ParseLogBuffer(0, logs) == 0);

    _CompareWithAdded(logs);
}

TEST_F(JournalLogBufferIntegrationTest, ParseLogBuffer)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::ParseLogBuffer");

    uint64_t offset = 0;
    while (offset + sizeof(BlockWriteDoneLog) < LOG_GROUP_SIZE)
    {
        LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
        context->SetBufferAllocated(offset, 0, 0);
        EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

        offset += context->GetLength();
        _AddToList(context);
    }

    _WaitForLogWriteDone(addedLogs.size());

    SimulateSPOR();

    LogList logs;
    EXPECT_TRUE(_ParseLogBuffer(0, logs) == 0);

    _CompareWithAdded(logs);
}

TEST_F(JournalLogBufferIntegrationTest, WriteInvalidLogType)
{
    POS_TRACE_DEBUG(9999, "JournalLogBufferIntegrationTest::WriteInvalidLogType");

    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());

    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* typePtr = reinterpret_cast<int*>(&(log->type));
        *typePtr = -1;

        context->SetBufferAllocated(0, 0, 0);
        EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) != 0);
        EXPECT_TRUE(groupLogs.IsEmpty() == true);
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

    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());

    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* markPtr = reinterpret_cast<int*>(&(log->mark));
        *markPtr = 0;

        context->SetBufferAllocated(0, 0, 0);
        EXPECT_TRUE(logBuffer->WriteLog(context) == 0);

        _WaitForLogWriteDone(1);

        LogList groupLogs;
        EXPECT_TRUE(_ParseLogBuffer(0, groupLogs) == 0);
        EXPECT_TRUE(groupLogs.IsEmpty() == true);
    }
    else
    {
        delete context;
        FAIL();
    }

    delete context;
}
} // namespace pos
