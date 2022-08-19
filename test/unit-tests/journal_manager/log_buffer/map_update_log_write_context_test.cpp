#include "src/journal_manager/log_buffer/map_update_log_write_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"

using testing::NiceMock;
using testing::Return;
namespace pos
{
TEST(MapUpdateLogWriteContext, GetDirtyList_testIfExecutedSuccessfully)
{
    // Given
    MapList expectDirtyMap;
    expectDirtyMap.emplace(1);
    MapUpdateLogWriteContext mapUpdatedLogWriteContext(expectDirtyMap, nullptr, nullptr);

    // When
    MapList result = mapUpdatedLogWriteContext.GetDirtyList();

    // Then
    EXPECT_EQ(expectDirtyMap, result);
}

TEST(MapUpdateLogWriteContext, LogWriteDone_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockEvent>* clientCallback = new NiceMock<MockEvent>;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    MapList dirtyMap;
    MapUpdateLogWriteContext mapUpdatedLogWriteContext(dirtyMap, EventSmartPtr(clientCallback), &notifier);

    // When, Then
    bool retResult = true;
    EXPECT_CALL(*clientCallback, Execute).WillOnce(Return(retResult));
    EXPECT_CALL(notifier, NotifyLogFilled);

    mapUpdatedLogWriteContext.IoDone();
}

} // namespace pos
