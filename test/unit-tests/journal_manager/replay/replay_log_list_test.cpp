#include "src/journal_manager/replay/replay_log_list.h"

#include <gtest/gtest.h>

#include <vector>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/journal_manager/log/log_handler_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplayLogList, ReplayLogList_testIfConstructedSuccessfully)
{
    ReplayLogList logList;

    {
        ReplayLogList* logList = new ReplayLogList();
        delete logList;
    }
}

TEST(ReplayLogList, AddLog_testIfMapUpdatedLogIsAdded)
{
    uint64_t seqNum = 10;
    NiceMock<MockLogHandlerInterface> log;
    ON_CALL(log, GetSeqNum).WillByDefault(Return(10));

    ReplayLogList logList;
    logList.Init(2);

    int logGroupId = 0;
    logList.AddLog(logGroupId, &log);

    auto replayLogs = std::move(logList.PopReplayLogGroup());
    EXPECT_EQ(replayLogs.size(), 1);
    EXPECT_EQ(replayLogs.front().time, 0);
    EXPECT_EQ(replayLogs.front().log, &log);
    EXPECT_EQ(replayLogs.front().segInfoFlushed, false);
}

TEST(ReplayLogList, AddLog_testIfLogsAddedToOneLogGroup)
{
    uint64_t seqNum = 5;
    int numLogs = 10;
    NiceMock<MockLogHandlerInterface> log[numLogs];
    for (int count = 0; count < numLogs; count++)
    {
        ON_CALL(log[count], GetSeqNum).WillByDefault(Return(seqNum));
    }

    ReplayLogList logList;
    logList.Init(2);
    int logGroupId = 0;
    for (int count = 0; count < numLogs; count++)
    {
        logList.AddLog(logGroupId, &log[count]);
    }

    auto replayLogs = logList.PopReplayLogGroup();
    EXPECT_EQ(replayLogs.size(), numLogs);

    for (int count = 0; count < numLogs; count++)
    {
        EXPECT_EQ(replayLogs[count].log, &(log[count]));
        EXPECT_EQ(replayLogs[count].time, count);
        EXPECT_EQ(replayLogs[count].segInfoFlushed, false);
    }
}

TEST(ReplayLogList, AddLog_testIfLogsAddedToTwoLogGroups)
{
    // Given
    int numLogsPerGroup = 10;
    NiceMock<MockLogHandlerInterface> log[2][numLogsPerGroup];

    for (int count = 0; count < numLogsPerGroup; count++)
    {
        ON_CALL(log[0][count], GetSeqNum).WillByDefault(Return(0));
        ON_CALL(log[1][count], GetSeqNum).WillByDefault(Return(1));
    }

    // When: logs are added to the list
    ReplayLogList logList;
    logList.Init(2);
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        logList.AddLog(0, &log[0][count]);
    }
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        logList.AddLog(1, &log[1][count]);
    }

    // Then
    auto replayLogs = logList.PopReplayLogGroup();
    EXPECT_EQ(replayLogs.size(), numLogsPerGroup * 2);

    for (int count = 0; count < numLogsPerGroup; count++)
    {
        EXPECT_EQ(replayLogs[count].log, &(log[0][count]));
        EXPECT_EQ(replayLogs[count].time, count);
        EXPECT_EQ(replayLogs[count].segInfoFlushed, false);
    }

    for (int count = 0; count < numLogsPerGroup; count++)
    {
        EXPECT_EQ(replayLogs[numLogsPerGroup + count].log, &(log[1][count]));
        EXPECT_EQ(replayLogs[numLogsPerGroup + count].time, numLogsPerGroup + count);
        EXPECT_EQ(replayLogs[numLogsPerGroup + count].segInfoFlushed, false);
    }
}

TEST(ReplayLogList, AddLog_testIfVolumeDeletingLogsAreAdded)
{
    // Given
    uint64_t seqNum = 5;
    int numLogs = 10;
    std::vector<NiceMock<MockLogHandlerInterface>*> logs;
    for (int count = 0; count < numLogs; count++)
    {
        logs.push_back(new NiceMock<MockLogHandlerInterface>);

        ON_CALL(*(logs.back()), GetSeqNum).WillByDefault(Return(seqNum));
        ON_CALL(*(logs.back()), GetType).WillByDefault(Return(LogType::VOLUME_DELETED));
    }

    // When
    ReplayLogList logList;
    logList.Init(2);
    for (int count = 0; count < numLogs; count++)
    {
        logList.AddLog(0, logs[count]);
    }

    // Then
    auto actual = logList.GetDeletingLogs();
    EXPECT_EQ(actual.size(), numLogs);

    for (int count = 0; count < numLogs; count++)
    {
        EXPECT_EQ(actual[count].log, logs[count]);
        EXPECT_EQ(actual[count].time, count);
    }
}

TEST(ReplayLogList, IsEmpty_testIfReturnTrueWhenNoLogsAreAdded)
{
    ReplayLogList logList;
    logList.Init(2);
    EXPECT_TRUE(logList.IsEmpty() == true);
}

TEST(ReplayLogList, IsEmpty_testIfReturnFalseWhenLogsAreAdded)
{
    NiceMock<MockLogHandlerInterface> log;
    ON_CALL(log, GetSeqNum).WillByDefault(Return(0));

    ReplayLogList logList;
    logList.Init(2);
    logList.AddLog(0, &log);

    EXPECT_TRUE(logList.IsEmpty() == false);
}

TEST(ReplayLogList, SetLogGroupFooter_testIfFooterUpdatedCorrectly)
{
    // Given
    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 5;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 2;

    // When
    ReplayLogList logList;
    logList.Init(2);

    int logGroupId = 0;
    logList.SetLogGroupFooter(logGroupId, footer);

    // Then
    auto actual = logList.GetLogGroupFooter(logGroupId);
    EXPECT_EQ(actual, footer);
}

TEST(ReplayLogList, EraseReplayLogGroup_testIfLogsAreErased)
{
    // Given
    ReplayLogList logList;
    logList.Init(2);

    int logGroupId = 0;
    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
        ON_CALL(*log, GetSeqNum).WillByDefault(Return(count % 2)); // return 0 or 1
        logList.AddLog(logGroupId, log);
    }

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 13;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 0;
    logList.SetLogGroupFooter(logGroupId, footer);

    // When
    int result = logList.EraseReplayLogGroup(logGroupId, footer.resetedSequenceNumber);
    EXPECT_EQ(result, EID(SUCCESS));

    // Then
    auto replayLogs = logList.PopReplayLogGroup();
    EXPECT_EQ(replayLogs.size(), 2); // seqNum 0 will be erased

    // Tear-Down
    for (auto replayLog : replayLogs)
    {
        delete replayLog.log;
    }
}

TEST(ReplayLogList, EraseReplayLogGroup_testIfLogsAreNotErased)
{
    // Given
    ReplayLogList logList;
    logList.Init(2);

    int logGroupId = 0;
    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
        ON_CALL(*log, GetSeqNum).WillByDefault(Return(1));
        logList.AddLog(logGroupId, log);
    }

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 13;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 0;
    logList.SetLogGroupFooter(logGroupId, footer);

    // When
    int result = logList.EraseReplayLogGroup(logGroupId, footer.resetedSequenceNumber);
    EXPECT_EQ(result, EID(SUCCESS));

    // Then
    auto replayLogs = logList.PopReplayLogGroup();
    EXPECT_EQ(replayLogs.size(), 5);

    // Tear-Down
    for (auto replayLog : replayLogs)
    {
        delete replayLog.log;
    }
}

TEST(ReplayLogList, EraseReplayLogGroup_testIfSeveralSequenceNumberSeen)
{
    // Given
    ReplayLogList logList;
    logList.Init(2);

    int logGroupId = 0;
    std::vector<NiceMock<MockLogHandlerInterface>*> logs;
    for (int count = 0; count < 5; count++)
    {
        NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
        ON_CALL(*log, GetSeqNum).WillByDefault(Return(count % 3)); // return 0 or 1 or 2

        if (count % 3 != 0) // log whose sequence number is 3 will be deleted in EraseReplayLogGroup
        {
            logs.push_back(log);
        }
        logList.AddLog(logGroupId, log);
    }

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 13;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 0;
    logList.SetLogGroupFooter(logGroupId, footer);

    // When
    int result = logList.EraseReplayLogGroup(logGroupId, footer.resetedSequenceNumber);

    // Then
    int expect = ERRID(JOURNAL_INVALID_LOG_FOUND);
    EXPECT_EQ(result, expect);

    // cleanup
    for (auto log : logs)
    {
        delete log;
    }
}

} // namespace pos
