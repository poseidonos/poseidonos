#include "src/journal_manager/log_write/log_write_handler.h"
#include "src/event_scheduler/meta_update_call_back.h"
#include "src/event_scheduler/callback.h"
#include "src/metadata/meta_event_factory.h"
#include "src/journal_manager/log_buffer/log_write_context_factory.h"
#include "src/mapper_service/mapper_service.h"
#include "src/metadata/block_map_update.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/log/waiting_log_list_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_write/buffer_offset_allocator_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_statistics_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_segment_ctx_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"

using testing::InSequence;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace pos
{
class LogWriteHandlerTestFixture : public ::testing::Test
{
public:
    LogWriteHandlerTestFixture(void)
    : bufferAllocator(nullptr),
      logBuffer(nullptr),
      config(nullptr),
      logWriteStats(nullptr),
      logWriteHandler(nullptr)
    {
    }

    virtual ~LogWriteHandlerTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        bufferAllocator = new NiceMock<MockBufferOffsetAllocator>;
        logBuffer = new NiceMock<MockJournalLogBuffer>;
        config = new NiceMock<MockJournalConfiguration>;

        logWriteStats = new NiceMock<MockLogWriteStatistics>;
        waitingList = new NiceMock<MockWaitingLogList>;

        logWriteHandler = new LogWriteHandler(logWriteStats, waitingList);

        ON_CALL(*bufferAllocator, GetLogGroupId).WillByDefault(Return(0));
    }

    virtual void
    TearDown(void)
    {
        delete bufferAllocator;
        delete logBuffer;
        delete config;

        delete logWriteHandler;
    }

protected:
    NiceMock<MockBufferOffsetAllocator>* bufferAllocator;
    NiceMock<MockJournalLogBuffer>* logBuffer;
    NiceMock<MockJournalConfiguration>* config;

    NiceMock<MockLogWriteStatistics>* logWriteStats;
    NiceMock<MockWaitingLogList>* waitingList;

    LogWriteHandler* logWriteHandler;
};

TEST_F(LogWriteHandlerTestFixture, BlockMapUpdateAddLog_testIfLogUpdatedSuccessfully)
{
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIStripeMap> stripeMap;
    NiceMock<MockISegmentCtx> segmentCtx;
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIArrayInfo> arrayInfo;

    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    StripeAddr stripeAddr = { .stripeLoc = IN_USER_AREA, .stripeId = 0};

    ON_CALL(*mockVolumeIo, GetOldLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(stripeAddr));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));

    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;
    logWriteContextFactory.Init(config, &notifier);

    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    int targetLogGroupId = 2;
    EXPECT_CALL(*bufferAllocator, GetLogGroupId).WillRepeatedly(Return(targetLogGroupId));

    int arrayId = 2;
    MapperServiceSingleton::Instance()->RegisterMapper("", arrayId, &vsaMap,
        nullptr, nullptr, nullptr, nullptr);
    MetaEventFactory metaEventFactory(&vsaMap, &stripeMap, &segmentCtx,
        &wbStripeAllocator, &contextManager, &arrayInfo);

    ON_CALL(*mockVolumeIo, GetArrayId).WillByDefault(Return(arrayId));

    CallbackSmartPtr blockMapUpdate =
        metaEventFactory.CreateBlockMapUpdateEvent(mockVolumeIoPtr);

    EXPECT_EQ(typeid(*blockMapUpdate.get()), typeid(BlockMapUpdate));

    LogWriteContext* logWriteContext =
        logWriteContextFactory.CreateBlockMapLogWriteContext(VolumeIoSmartPtr(mockVolumeIoPtr), blockMapUpdate);

    // When: Log is added
    EXPECT_TRUE(logWriteHandler->AddLog(logWriteContext) == 0);

    MetaUpdateCallback* metaUpdateCallback = dynamic_cast<MetaUpdateCallback*>(blockMapUpdate.get());
    int updatedLogGroupId = metaUpdateCallback->GetLogGroupId();
    EXPECT_EQ(targetLogGroupId, updatedLogGroupId);

    delete logWriteContext;
}

TEST_F(LogWriteHandlerTestFixture, Init_testIfStatsInitialized)
{
    // Given: Journal debug is configured
    EXPECT_CALL(*config, IsDebugEnabled).WillOnce(Return(true));

    // Then: LogWriteStatistics should be initialized
    EXPECT_CALL(*logWriteStats, Init);

    // When: LogWriteHandler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
}

TEST_F(LogWriteHandlerTestFixture, Init_testIfStatsNotInitialized)
{
    // Given: Journal debug is configured
    EXPECT_CALL(*config, IsDebugEnabled).WillOnce(Return(false));

    // When: LogWriteHandler is initialized without initializing LogWriteStatistics
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testIfLogWritten)
{
    // Given: Log write handler is initialized and context with dummy value is given
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Waiting list is empty and buffer allocation succeeds
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(0));

    // Then: Log should be successfully requested to be written to log buffer
    EXPECT_CALL(*logBuffer, WriteLog).WillOnce(Return(0));

    // When: Log is added
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) == 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testBufferAllocFailedWithPositiveReturn)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Waiting list is empty and buffer allocation does not succeed
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(EID(JOURNAL_LOG_GROUP_FULL)));

    // When: Log is added
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) > 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testBufferAllocFailedWithNegativeReturn)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Waiting list is empty, log is added and buffer allocation fails and
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(-1 * EID(JOURNAL_INVALID_SIZE_LOG_REQUESTED)));

    // Then: Add log should be failed with error code
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) < 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testIfContextCleanedUpWhenWriteLogFailed)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Waiting list is empty, buffer allocation succeed, and write log fails
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(0));
    EXPECT_CALL(*logBuffer, WriteLog).WillOnce(Return(-1));

    EXPECT_CALL(*bufferAllocator, LogWriteCanceled).Times(1);

    // When: Log is added
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) < 0);

    // Then: The context should be deleted by LogWriteHandler
}

TEST_F(LogWriteHandlerTestFixture, AddLogToWaitingList_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockLogWriteContext> context;
    EXPECT_CALL(*waitingList, AddToList(&context));
    logWriteHandler->AddLogToWaitingList(&context);
}

TEST_F(LogWriteHandlerTestFixture, LogWriteDone_testIfCallbackExecuted)
{
    // Given: Log write handler is initialized, waiting list is empty,
    // LogWriteStatistics update failed
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
    EXPECT_CALL(*waitingList, GetWaitingIo).WillOnce(Return(nullptr));
    EXPECT_CALL(*logWriteStats, UpdateStatus).WillOnce(Return(false));

    // Then: The written context callback should be called
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_CALL(*context, IoDone);

    // When: Log write is done
    logWriteHandler->LogWriteDone(context);

    // Then: The written context should be deleted by LogWriteHandler
}

TEST_F(LogWriteHandlerTestFixture, LogWriteDone_testIfLogWriteStatisticsUpdated)
{
    // Given: Log write handler is initialized, waiting list is empty, and
    // LogWriteContext is given
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
    EXPECT_CALL(*waitingList, GetWaitingIo).WillOnce(Return(nullptr));

    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;

    // Then: The completed log should be added to the statistics
    // and context callback should be called in this order
    {
        InSequence s;

        EXPECT_CALL(*logWriteStats, UpdateStatus).WillOnce(Return(true));
        EXPECT_CALL(*context, IoDone);
        EXPECT_CALL(*logWriteStats, AddToList);
    }

    // When: Log write is done
    logWriteHandler->LogWriteDone(context);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, LogWriteDone_testIfWaitingListIsRestarted)
{
    // Given: Log write handler is initialized, and log write context is given
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;

    // When: Waiting log list is not empty, and statistics update is failed
    NiceMock<MockLogWriteContext>* waitingContext = new NiceMock<MockLogWriteContext>;
    EXPECT_CALL(*waitingList, GetWaitingIo).WillOnce(Return(waitingContext));

    EXPECT_CALL(*logWriteStats, UpdateStatus).WillOnce(Return(false));

    // Then: The context callback should be called
    EXPECT_CALL(*context, IoDone);

    // Then: The waiting log should be started
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(0));
    EXPECT_CALL(*logBuffer, WriteLog).WillOnce(Return(0));

    // When: Log write is done
    logWriteHandler->LogWriteDone(context);

    // Then: The written context should be deleted by LogWriteHandler

    delete waitingContext;
}

TEST_F(LogWriteHandlerTestFixture, LogWriteDone_testIfExcutionSucessWhenIoFails)
{
    // Given: Log write handler is initialized, and log write context is given
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;

    // When: The context has an error
    EXPECT_CALL(*context, GetError).WillOnce(Return(-1));

    // Then: Log write stats should not be updatedm
    // and the context callback should be called
    EXPECT_CALL(*logWriteStats, UpdateStatus).Times(0);
    EXPECT_CALL(*context, IoDone);

    // When: Log write is done
    logWriteHandler->LogWriteDone(context);

    // Then: The written context should be deleted by LogWriteHandler
}

TEST_F(LogWriteHandlerTestFixture, LogFilled_testIfExecutionSuccess)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Log is filled
    MapList dummyDirty;
    logWriteHandler->LogFilled(0, dummyDirty);

    // Then: LogFilled should be executed successfully
}

TEST_F(LogWriteHandlerTestFixture, LogBufferReseted_testIfWaitingListIsRestarted)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config, nullptr);

    // When: Waiting log list is not empty
    NiceMock<MockLogWriteContext>* waitingContext = new NiceMock<MockLogWriteContext>;
    EXPECT_CALL(*waitingList, GetWaitingIo).WillOnce(Return(waitingContext));

    // Then: The waiting log should be started
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(0));
    EXPECT_CALL(*logBuffer, WriteLog).WillOnce(Return(0));

    // When: Log buffer reset is done
    logWriteHandler->LogBufferReseted(0);

    delete waitingContext;
}
} // namespace pos
