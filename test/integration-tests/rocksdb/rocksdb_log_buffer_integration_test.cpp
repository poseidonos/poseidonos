#include "rocksdb_log_buffer_integration_test.h"

#include <time.h>

#include <experimental/filesystem>

#include "gtest/gtest.h"
#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/log_buffer_parser.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"
#include "src/rocksdb_log_buffer/rocksdb_log_buffer.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"

namespace pos
{
using ::testing::Return;

RocksDBLogBufferIntegrationTest::RocksDBLogBufferIntegrationTest(void)
: journalRocks(nullptr)
{
}

RocksDBLogBufferIntegrationTest::~RocksDBLogBufferIntegrationTest(void)
{
}

void
RocksDBLogBufferIntegrationTest::SetUp(void)
{
    // remove rocksdb log files by removing temporary directory if exist
    std::string targetDirName = "/etc/pos/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(targetDirName);
    std::string SPORDirectory = "/etc/pos/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);

    numLogsWritten = 0;
    addedLogs.clear();

    ON_CALL(config, IsEnabled).WillByDefault(Return(true));
    ON_CALL(config, GetLogBufferSize).WillByDefault(Return(LOG_BUFFER_SIZE));
    ON_CALL(config, GetLogGroupSize).WillByDefault(Return(LOG_GROUP_SIZE));

    factory.Init(&config, new LogBufferWriteDoneNotifier(), new CallbackSequenceController());
    journalRocks = new RocksDBLogBuffer(GetLogDirName());
    journalRocks->Init(&config, &factory, 0, nullptr);

    uint64_t logBufferSize = LOG_BUFFER_SIZE;
    journalRocks->Create(logBufferSize);
    journalRocks->SyncResetAll();
}

void
RocksDBLogBufferIntegrationTest::TearDown(void)
{
    for (auto it : addedLogs)
    {
        delete it;
    }
    addedLogs.clear();

    // Teardown : remove rocksdb log files by removing temporary directory.
    std::string targetDirName = "/etc/pos/" + GetLogDirName() + "_RocksJournal";
    int ret = std::experimental::filesystem::remove_all(targetDirName);
    EXPECT_TRUE(ret >= 1);

    std::string SPORDirectory = "/etc/pos/SPOR" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::remove_all(SPORDirectory);

    journalRocks->Close();
}

LogWriteContext*
RocksDBLogBufferIntegrationTest::_CreateContextForBlockWriteDoneLog(void)
{
    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 0, 0));
    volumeIo->SetSectorRba(0);
    volumeIo->SetVolumeId(TEST_VOLUME_ID);

    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateBlockMapLogWriteContext(volumeIo, callback);
    return context;
}

LogWriteContext*
RocksDBLogBufferIntegrationTest::_CreateContextForStripeMapUpdatedLog(void)
{
    NiceMock<MockStripe>* stripe = new NiceMock<MockStripe>();
    StripeAddr oldAddr =
        {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = 0};
    EventSmartPtr callback(new LogBufferWriteDone());

    LogWriteContext* context =
        factory.CreateStripeMapLogWriteContext(stripe, oldAddr, callback);

    return context;
}

LogWriteContext*
RocksDBLogBufferIntegrationTest::_CreateContextForGcBlockWriteDoneLog(void)
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

    return context;
}

LogWriteContext*
RocksDBLogBufferIntegrationTest::_CreateContextForGcStripeFlushedLog(void)
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

    return context;
}

int
RocksDBLogBufferIntegrationTest::_ParseLogBuffer(int groupId, LogList& groupLogs)
{
    void* buffer = calloc(LOG_GROUP_SIZE, sizeof(char));

    EXPECT_TRUE(journalRocks->ReadLogBuffer(groupId, buffer) == 0);

    LogBufferParser parser;
    int result = parser.GetLogs(buffer, LOG_GROUP_SIZE, groupLogs);

    free(buffer);
    return result;
}

void
RocksDBLogBufferIntegrationTest::_CompareWithAdded(LogList& logList)
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
RocksDBLogBufferIntegrationTest::_AddToList(LogWriteContext* context)
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

int
RocksDBLogBufferIntegrationTest::_PrepareLogBuffer(void)
{
    int result = 0;
    if (journalRocks->DoesLogFileExist() == true)
    {
        uint64_t logBufferSize = 0;
        result = journalRocks->Open(logBufferSize);
        if (result < 0)
        {
            return result;
        }
        EXPECT_CALL(config, GetLogBufferSize).WillRepeatedly(Return(logBufferSize));
        EXPECT_CALL(config, GetLogGroupSize).WillRepeatedly(Return(logBufferSize / NUM_LOG_GROUPS));
    }
    else
    {
        result = journalRocks->Create(LOG_BUFFER_SIZE);
    }
    return result;
}

void
RocksDBLogBufferIntegrationTest::SimulateSPOR(void)
{
    // To Simulate SPOR, copy rocksdb data to another directory at any time which is similar to closing rocksdb abrubtly before closing db.
    std::string SPOR = "SPOR" + GetLogDirName();
    std::string SPORDirectory = "/etc/pos/" + SPOR + "_RocksJournal";
    std::string targetDirName = "/etc/pos/" + GetLogDirName() + "_RocksJournal";
    std::experimental::filesystem::copy(targetDirName, SPORDirectory);

    // Open abrubtly closed rocksDB (Copied rocksdb)
    delete journalRocks;
    journalRocks = new RocksDBLogBuffer(SPOR);
    _PrepareLogBuffer();
    journalRocks->Init(&config, &factory, 0, nullptr);
}

TEST_F(RocksDBLogBufferIntegrationTest, CreateAndClose)
{
    // Given : array name and When JournalRocks opened
    RocksDBLogBuffer journalRocks("OpenAndClose");
    uint64_t logBufferSize = LOG_BUFFER_SIZE;
    int createStatus = journalRocks.Create(logBufferSize);
    std::string targetDirName = "/etc/pos/OpenAndClose_RocksJournal";
    // Then : Directory is exist, open status is success (0) and isOpened variable is true
    EXPECT_EQ(std::experimental::filesystem::exists(targetDirName), true);
    EXPECT_EQ(createStatus, 0);
    EXPECT_EQ(journalRocks.IsOpened(), true);

    // When : JournalRocks closed
    int closedStatus = journalRocks.Close();
    // Then : isOpened variable is false and closed status is success (0)
    EXPECT_EQ(journalRocks.IsOpened(), false);
    EXPECT_EQ(closedStatus, 0);
    // Teardown : remove rocksdb log files by removing temporary directory.
    int ret = std::experimental::filesystem::remove_all(targetDirName);
    EXPECT_TRUE(ret >= 1);
}

TEST_F(RocksDBLogBufferIntegrationTest, WriteAndVerify)
{
    uint64_t offset = 0;

    // Write block write done log
    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write stripe map updated log
    context = _CreateContextForStripeMapUpdatedLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write gc block write done log
    context = _CreateContextForGcBlockWriteDoneLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    // Write gc stripe flushed log
    context = _CreateContextForGcStripeFlushedLog();
    context->SetBufferAllocated(offset, 0, 0);
    EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

    offset += context->GetLength();
    _AddToList(context);

    EXPECT_EQ(4, addedLogs.size());

    SimulateSPOR();

    LogList logs;
    EXPECT_TRUE(_ParseLogBuffer(0, logs) == 0);

    _CompareWithAdded(logs);
}

TEST_F(RocksDBLogBufferIntegrationTest, ParseLogBuffer)
{
    uint64_t offset = 0;
    while (offset + sizeof(BlockWriteDoneLog) < LOG_GROUP_SIZE)
    {
        LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
        context->SetBufferAllocated(offset, 0, 0);
        EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

        offset += context->GetLength();
        _AddToList(context);
    }

    SimulateSPOR();

    LogList logs;
    EXPECT_TRUE(_ParseLogBuffer(0, logs) == 0);

    _CompareWithAdded(logs);
}

TEST_F(RocksDBLogBufferIntegrationTest, WriteInvalidLogType)
{
    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());
    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* typePtr = reinterpret_cast<int*>(&(log->type));
        *typePtr = -1;

        context->SetBufferAllocated(0, 0, 0);
        EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

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

TEST_F(RocksDBLogBufferIntegrationTest, WriteWithoutMark)
{
    LogWriteContext* context = _CreateContextForBlockWriteDoneLog();
    BlockWriteDoneLogHandler* blockLog =
        dynamic_cast<BlockWriteDoneLogHandler*>(context->GetLog());

    if (blockLog != nullptr)
    {
        Log* log = reinterpret_cast<Log*>(blockLog->GetData());
        int* markPtr = reinterpret_cast<int*>(&(log->mark));
        *markPtr = 0;

        context->SetBufferAllocated(0, 0, 0);
        EXPECT_TRUE(journalRocks->WriteLog(context) == 0);

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
