#include "src/journal_manager/log_buffer/buffer_write_done_notifier.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"

using testing::NiceMock;

namespace pos
{
TEST(LogBufferWriteDoneNotifier, LogBufferWriteDoneNotifier_testIfSubscriberIsEmpty)
{
    // Given
    LogBufferWriteDoneNotifier notifier;

    // When, Then
    int expected = 0;
    std::vector<LogBufferWriteDoneEvent*> actual = notifier.GetSubscribers();
    EXPECT_EQ(expected, actual.size());
}

TEST(LogBufferWriteDoneNotifier, Register_testIfExecutedSuccessfully)
{
    // Given
    LogBufferWriteDoneNotifier notifier;
    NiceMock<MockLogBufferWriteDoneEvent> event;

    std::vector<LogBufferWriteDoneEvent*> expected;

    // When
    notifier.Register(&event);
    expected.push_back(&event);

    // Then
    std::vector<LogBufferWriteDoneEvent*> actual = notifier.GetSubscribers();
    EXPECT_EQ(expected, actual);
}

TEST(LogBufferWriteDoneNotifier, NotifyLogFilled_testIfExecutedSuccessfully)
{
    // Given
    LogBufferWriteDoneNotifier notifier;
    NiceMock<MockLogBufferWriteDoneEvent> event;

    notifier.Register(&event);

    // When, Then
    int logGroupId = 0;
    MapList dirty;
    EXPECT_CALL(event, LogFilled(logGroupId, dirty));
    notifier.NotifyLogFilled(logGroupId, dirty);
}

TEST(LogBufferWriteDoneNotifier, NotifyLogBufferReseted_testIfExecutedSuccessfully)
{
    // Given
    LogBufferWriteDoneNotifier notifier;
    NiceMock<MockLogBufferWriteDoneEvent> event;

    notifier.Register(&event);

    // When, Then
    int logGroupId = 0;
    EXPECT_CALL(event, LogBufferReseted(logGroupId));
    notifier.NotifyLogBufferReseted(logGroupId);
}

} // namespace pos
