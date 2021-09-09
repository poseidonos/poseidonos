#include "src/journal_manager/replay/replay_log_list.h"

#include <gtest/gtest.h>

#include <vector>

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
    logList.AddLog(&log);

    ReplayLogGroup logGroup = logList.PopReplayLogGroup();
    EXPECT_EQ(logGroup.seqNum, seqNum);
    EXPECT_EQ(logGroup.logs.size(), 1);
    EXPECT_EQ(logGroup.logs.front().time, 0);
    EXPECT_EQ((logGroup.logs.front().log), &log);
    EXPECT_EQ(logGroup.logs.front().segInfoFlushed, false);
    EXPECT_EQ(logGroup.isFooterValid, false);
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
    for (int count = 0; count < numLogs; count++)
    {
        logList.AddLog(&log[count]);
    }

    ReplayLogGroup logGroup = logList.PopReplayLogGroup();
    EXPECT_EQ(logGroup.seqNum, seqNum);
    EXPECT_EQ(logGroup.logs.size(), numLogs);

    auto logs = logGroup.logs;
    for (int count = 0; count < numLogs; count++)
    {
        EXPECT_EQ(logs[count].log, &(log[count]));
        EXPECT_EQ(logs[count].time, count);
        EXPECT_EQ(logs[count].segInfoFlushed, false);
    }
    EXPECT_EQ(logGroup.isFooterValid, false);
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
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        logList.AddLog(&log[0][count]);
    }
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        logList.AddLog(&log[1][count]);
    }

    // Then
    ReplayLogGroup logGroup0 = logList.PopReplayLogGroup();
    EXPECT_EQ(logGroup0.seqNum, 0);
    EXPECT_EQ(logGroup0.logs.size(), numLogsPerGroup);
    auto logs0 = logGroup0.logs;
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        EXPECT_EQ(logs0[count].log, &(log[0][count]));
        EXPECT_EQ(logs0[count].time, count);
        EXPECT_EQ(logs0[count].segInfoFlushed, false);
    }
    EXPECT_EQ(logGroup0.isFooterValid, false);

    ReplayLogGroup logGroup1 = logList.PopReplayLogGroup();
    EXPECT_EQ(logGroup1.seqNum, 1);
    EXPECT_EQ(logGroup1.logs.size(), numLogsPerGroup);
    auto logs1 = logGroup1.logs;
    for (int count = 0; count < numLogsPerGroup; count++)
    {
        EXPECT_EQ(logs1[count].log, &(log[1][count]));
        EXPECT_EQ(logs1[count].time, numLogsPerGroup + count);
        EXPECT_EQ(logs1[count].segInfoFlushed, false);
    }
    EXPECT_EQ(logGroup1.isFooterValid, false);
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
    for (int count = 0; count < numLogs; count++)
    {
        logList.AddLog(logs[count]);
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
    EXPECT_TRUE(logList.IsEmpty() == true);
}

TEST(ReplayLogList, IsEmpty_testIfReturnFalseWhenLogsAreAdded)
{
    NiceMock<MockLogHandlerInterface> log;
    ON_CALL(log, GetSeqNum).WillByDefault(Return(0));

    ReplayLogList logList;
    logList.AddLog(&log);

    EXPECT_TRUE(logList.IsEmpty() == false);
}

TEST(ReplayLogList, SetLogGroupFooter_testIfFooterUpdatedCorrectly)
{
    // Given
    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 5;
    uint64_t seqNum = 10;
    int numLogs = 10;
    NiceMock<MockLogHandlerInterface> log[numLogs];
    for (int count = 0; count < numLogs; count++)
    {
        ON_CALL(log[count], GetSeqNum).WillByDefault(Return(seqNum));
    }

    // When
    ReplayLogList logList;
    for (int count = 0; count < numLogs; count++)
    {
        logList.AddLog(&log[count]);
    }
    logList.SetLogGroupFooter(seqNum, footer);

    // Then
    ReplayLogGroup logGroup = logList.PopReplayLogGroup();
    EXPECT_EQ(logGroup.seqNum, seqNum);
    EXPECT_EQ(logGroup.logs.size(), numLogs);
    auto logs = logGroup.logs;
    for (int count = 0; count < numLogs; count++)
    {
        EXPECT_EQ(logs[count].log, &(log[count]));
        EXPECT_EQ(logs[count].time, count);
        EXPECT_EQ(logs[count].segInfoFlushed, false);
    }
    EXPECT_EQ(logGroup.isFooterValid, true);
    EXPECT_EQ(logGroup.footer, footer);
}

} // namespace pos
