#include "src/journal_manager/log/log_list.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log/log_handler_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(LogList, LogList_testIfConstructedSuccessfully)
{
    LogList list;

    LogList* listInHeap = new LogList();
    delete listInHeap;
}

TEST(LogList, Reset_testIfLogsAreDeleted)
{
    LogList list;

    for (int count = 0; count < 10; count++)
    {
        NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
        list.AddLog(0, log);
    }

    list.Reset();
    EXPECT_TRUE(list.IsEmpty());
}

TEST(LogList, AddLog_testIfLogIsAddedSuccessfully)
{
    LogList list;
    NiceMock<MockLogHandlerInterface>* log = new NiceMock<MockLogHandlerInterface>;
    list.AddLog(0, log);

    EXPECT_EQ(list.GetLogs().size(), 1);
    EXPECT_EQ(list.GetLogs().front(), log);
}

TEST(LogList, SetLogGroupFooter_testIfExecutedSuccessfully)
{
    LogList list;
    LogGroupFooter footer;
    list.SetLogGroupFooter(0, footer);
}
} // namespace pos
