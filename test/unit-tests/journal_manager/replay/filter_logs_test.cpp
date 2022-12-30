#include "src/journal_manager/replay/filter_logs.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_log_list_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FilterLogs, Start_testIfItReturnsNonZeroCodeWhenLogListIsEmpty)
{
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);

    EXPECT_CALL(logList, IsEmpty).WillOnce(Return(true));

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(JOURNAL_REPLAY_STOPPED));
}

TEST(FilterLogs, Start_testLogGroupStatus0)
{
    // Log Group Status 0
    // Log group 0: Allocated, log group 1: Not used

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2]; // boths invalid
    footers[0].isReseted = false;
    footers[0].lastCheckpointedSeginfoVersion = UINT32_MAX;
    footers[0].resetedSequenceNumber = UINT32_MAX;

    footers[1].isReseted = false;
    footers[1].lastCheckpointedSeginfoVersion = UINT32_MAX;
    footers[1].resetedSequenceNumber = UINT32_MAX;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup).Times(0);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}

TEST(FilterLogs, Start_testLogGroupStatus1)
{
    // Log Group Status 1
    // Log group 0: Checkpointed, log group 1: Allocated

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2];
    footers[0].isReseted = true;
    footers[0].lastCheckpointedSeginfoVersion = 0;
    footers[0].resetedSequenceNumber = 0;

    footers[1].isReseted = false;
    footers[1].lastCheckpointedSeginfoVersion = UINT32_MAX;
    footers[1].resetedSequenceNumber = UINT32_MAX;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup(0, 0)).Times(1);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}

TEST(FilterLogs, Start_testLogGroupStatus2)
{
    // Log Group Status 2
    // Log group 0: Checkpointed, log group 1: Checkpoint started

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2];
    footers[0].isReseted = true;
    footers[0].lastCheckpointedSeginfoVersion = 0;
    footers[0].resetedSequenceNumber = 0;

    footers[1].isReseted = false;
    footers[1].lastCheckpointedSeginfoVersion = 1;
    footers[1].resetedSequenceNumber = UINT32_MAX;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup(0, 0)).Times(1);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}

TEST(FilterLogs, Start_testLogGroupStatus3)
{
    // Log Group Status 3
    // Log group 0: Re-allocated, log group 1: Checkpoint completed

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2];
    footers[0].isReseted = true;
    footers[0].lastCheckpointedSeginfoVersion = 0;
    footers[0].resetedSequenceNumber = 0;

    footers[1].isReseted = true;
    footers[1].lastCheckpointedSeginfoVersion = 1;
    footers[1].resetedSequenceNumber = 1;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup(0, 0)).Times(1);
    EXPECT_CALL(logList, EraseReplayLogGroup(1, 1)).Times(1);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}

TEST(FilterLogs, Start_testLogGroupStatus4)
{
    // Log Group Status 4
    // Log group 0: Checkpoint started, log group 1: Re-allocated
    // assume allocator context is not flushed yet

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2];
    footers[0].isReseted = false;
    footers[0].lastCheckpointedSeginfoVersion = 3;
    footers[0].resetedSequenceNumber = 3;

    // Footer 1 is invalid
    footers[1].isReseted = true;
    footers[1].lastCheckpointedSeginfoVersion = 4;
    footers[1].resetedSequenceNumber = 4;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));
    ON_CALL(contextManager, GetStoredContextVersion).WillByDefault(Return(3));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup(0, 3)).Times(1);
    EXPECT_CALL(logList, EraseReplayLogGroup(1, 4)).Times(1);

    EXPECT_CALL(logList, SetSegInfoFlushed).Times(0);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}

TEST(FilterLogs, Start_testLogGroupStatus5)
{
    // Log Group Status 5
    // Log group 0: Checkpoint started, log group 1: Re-allocated
    // assume allocator context is flushed

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> logList;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FilterLogs filterLogsTask(&config, logList, &contextManager, &reporter);
    ON_CALL(logList, IsEmpty).WillByDefault(Return(false));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    LogGroupFooter footers[2];
    footers[0].isReseted = false;
    footers[0].lastCheckpointedSeginfoVersion = 3;
    footers[0].resetedSequenceNumber = 3;

    // Footer 1 is invalid
    footers[1].isReseted = true;
    footers[1].lastCheckpointedSeginfoVersion = 4;
    footers[1].resetedSequenceNumber = 4;

    ON_CALL(logList, GetLogGroupFooter(0)).WillByDefault(Return(footers[0]));
    ON_CALL(logList, GetLogGroupFooter(1)).WillByDefault(Return(footers[1]));
    ON_CALL(contextManager, GetStoredContextVersion).WillByDefault(Return(4));

    // It should not erase any logs
    EXPECT_CALL(logList, EraseReplayLogGroup(0, 3)).Times(1);
    EXPECT_CALL(logList, EraseReplayLogGroup(1, 4)).Times(1);

    EXPECT_CALL(logList, SetSegInfoFlushed(0)).Times(1);

    int ret = filterLogsTask.Start();
    EXPECT_EQ(ret, EID(SUCCESS));
}
} // namespace pos
