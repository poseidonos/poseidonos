#include "src/journal_manager/log_buffer/log_write_io_context.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/journal_manager/log/log_handler_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"

using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(LogWriteIoContext, GetLog_testIfLogInLogWriteContextIsReturned)
{
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockLogWriteContext> context;

    LogWriteIoContext ioContext(&context, &notifier);

    NiceMock<MockLogHandlerInterface> log;
    EXPECT_CALL(context, GetLog).WillOnce(Return(&log));

    EXPECT_EQ(ioContext.GetLog(), &log);
}

TEST(LogWriteIoContext, GetLog_testIfLogGroupIdInLogWriteContextIsReturned)
{
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockLogWriteContext> context;

    LogWriteIoContext ioContext(&context, &notifier);

    EXPECT_CALL(context, GetLogGroupId).WillOnce(Return(6));

    EXPECT_EQ(ioContext.GetLogGroupId(), 6);
}

TEST(LogWriteIoContext, IoDone_testIfNotifierIsCalled)
{
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockLogWriteContext> context;
    auto callbackEvent = std::make_shared<MockEvent>();
    ON_CALL(context, GetCallback).WillByDefault(Return(callbackEvent));
    ON_CALL(context, GetLogGroupId).WillByDefault(Return(3));

    LogWriteIoContext ioContext(&context, &notifier);

    const MapList dummy;
    EXPECT_CALL(context, GetDirtyMapList).WillOnce(ReturnRef(dummy));

    {
        InSequence s;

        EXPECT_CALL(*callbackEvent.get(), Execute).WillOnce(Return(true));
        EXPECT_CALL(notifier, NotifyLogFilled(3, dummy)).Times(1);
    }

    ioContext.IoDone();
}

} // namespace pos
