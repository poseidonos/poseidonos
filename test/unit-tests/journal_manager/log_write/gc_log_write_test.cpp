#include "src/journal_manager/log_write/gc_log_write.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

namespace pos
{
using ::testing::NiceMock;
using ::testing::Return;

TEST(GcLogWrite, DoSpecificJob_testIfCallbackExecutedWhenNoBlockMapProvided)
{
    std::vector<LogWriteContext*> emptyBlockContexts;
    NiceMock<LogWriteHandler> logWriteHandler;

    NiceMock<MockEvent>* mock = new NiceMock<MockEvent>();
    EXPECT_CALL(*mock, Execute).WillOnce(Return(false)).WillOnce(Return(true));
    EventSmartPtr callback(mock);

    GcLogWrite gcLogWrite(emptyBlockContexts, callback, &logWriteHandler);

    EXPECT_EQ(gcLogWrite.Execute(), false);
    EXPECT_EQ(gcLogWrite.Execute(), true);
}

TEST(GcLogWrite, DoSpecificJob_testIfAllLogsAreAdded)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;
    EventSmartPtr callback(new MockEvent());

    std::vector<LogWriteContext*> blockContexts;
    for (int count = 0; count < 5; count++)
    {
        blockContexts.push_back(new NiceMock<MockLogWriteContext>);
    }
    EXPECT_CALL(logWriteHandler, AddLog).WillRepeatedly(Return(0));

    GcLogWrite gcLogWrite(blockContexts, callback, &logWriteHandler);
    EXPECT_EQ(gcLogWrite.Execute(), true);

    for (auto context : blockContexts)
    {
        delete context;
    }
    blockContexts.clear();
}

TEST(GcLogWrite, DoSpecificJob_testIfLogWriteRetriedWhenAddLogFailsWithPositiveReturn)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;
    EventSmartPtr callback(new MockEvent());

    std::vector<LogWriteContext*> blockContexts;
    for (int count = 0; count < 3; count++)
    {
        blockContexts.push_back(new NiceMock<MockLogWriteContext>);
    }
    GcLogWrite gcLogWrite(blockContexts, callback, &logWriteHandler);

    EXPECT_CALL(logWriteHandler, AddLog)
        .WillOnce(Return(1000))
        .WillOnce(Return(0))
        .WillOnce(Return(1000)) // In first run
        .WillOnce(Return(0))
        .WillOnce(Return(0)); // In second run

    EXPECT_EQ(gcLogWrite.Execute(), false);
    EXPECT_EQ(gcLogWrite.Execute(), true);

    for (auto context : blockContexts)
    {
        delete context;
    }
    blockContexts.clear();
}

TEST(GcLogWrite, DoSpecificJob_testIfLogWriteRequestStoppedAndRetriedWhenAddLogFailsWithNegativeReturn)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;
    EventSmartPtr callback(new MockEvent());

    std::vector<LogWriteContext*> blockContexts;
    for (int count = 0; count < 3; count++)
    {
        blockContexts.push_back(new NiceMock<MockLogWriteContext>);
    }
    GcLogWrite gcLogWrite(blockContexts, callback, &logWriteHandler);

    EXPECT_CALL(logWriteHandler, AddLog)
        .WillOnce(Return(-1)) // In first run
        .WillOnce(Return(0))
        .WillOnce(Return(0))
        .WillOnce(Return(0)); // In second run

    EXPECT_EQ(gcLogWrite.Execute(), false);
    EXPECT_EQ(gcLogWrite.Execute(), true);

    for (auto context : blockContexts)
    {
        delete context;
    }
    blockContexts.clear();
}

} // namespace pos
