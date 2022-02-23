#include "src/journal_manager/log_write/log_write_handler.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/log/waiting_log_list_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_write/buffer_offset_allocator_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_statistics_mock.h"

using testing::InSequence;
using testing::NiceMock;
using testing::Return;

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

TEST_F(LogWriteHandlerTestFixture, Init_testIfStatsInitialized)
{
    // Given: Journal debug is configured
    EXPECT_CALL(*config, IsDebugEnabled).WillOnce(Return(true));

    // Then: LogWriteStatistics should be initialized
    EXPECT_CALL(*logWriteStats, Init);

    // When: LogWriteHandler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
}

TEST_F(LogWriteHandlerTestFixture, Init_testIfStatsNotInitialized)
{
    // Given: Journal debug is configured
    EXPECT_CALL(*config, IsDebugEnabled).WillOnce(Return(false));

    // When: LogWriteHandler is initialized without initializing LogWriteStatistics
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testIfLogWritten)
{
    // Given: Log write handler is initialized and context with dummy value is given
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // When: Waiting list is empty and buffer allocation succeeds
    EXPECT_CALL(*waitingList, AddToListIfNotEmpty).WillOnce(Return(false));
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // Then: Log should be requested to be written
    EXPECT_CALL(*waitingList, AddToList);

    // When: Waiting list is empty and buffer allocation does not succeed
    EXPECT_CALL(*waitingList, AddToListIfNotEmpty).WillOnce(Return(false));
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return((int)POS_EVENT_ID::JOURNAL_LOG_GROUP_FULL));

    // When: Log is added
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) == 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testBufferAllocFailedWithNegativeReturn)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // When: Waiting list is empty, log is added and buffer allocation fails and
    EXPECT_CALL(*waitingList, AddToListIfNotEmpty).WillOnce(Return(false));
    EXPECT_CALL(*bufferAllocator, AllocateBuffer).WillOnce(Return(-1 * (int)POS_EVENT_ID::JOURNAL_INVALID_SIZE_LOG_REQUESTED));

    // Then: Add log should be failed with error code
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) < 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testIfLogPendedWhenWaitingListNotEmpty)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // When: Waiting log list is not empty
    EXPECT_CALL(*waitingList, AddToListIfNotEmpty).WillOnce(Return(true));

    // When: Log should be added
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    EXPECT_TRUE(logWriteHandler->AddLog(context) == 0);

    delete context;
}

TEST_F(LogWriteHandlerTestFixture, AddLog_testIfContextCleanedUpWhenWriteLogFailed)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // When: Waiting list is empty, buffer allocation succeed, and write log fails
    EXPECT_CALL(*waitingList, AddToListIfNotEmpty).WillOnce(Return(false));
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);
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
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

    // When: Log is filled
    MapPageList dummyDirty;
    logWriteHandler->LogFilled(0, dummyDirty);

    // Then: LogFilled should be executed successfully
}

TEST_F(LogWriteHandlerTestFixture, LogBufferReseted_testIfWaitingListIsRestarted)
{
    // Given: Log write handler is initialized
    logWriteHandler->Init(bufferAllocator, logBuffer, config);

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
