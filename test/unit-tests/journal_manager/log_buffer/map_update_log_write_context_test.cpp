#include "src/journal_manager/log_buffer/map_update_log_write_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"

using testing::InSequence;
using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

namespace pos
{
TEST(MapUpdateLogWriteContext, IoDone_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockLogWriteContext>* context = new NiceMock<MockLogWriteContext>;
    NiceMock<MockEvent>* clientCallback = new NiceMock<MockEvent>;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    MapList dummy;
    ON_CALL(*context, GetCallback).WillByDefault(Return(EventSmartPtr(clientCallback)));
    ON_CALL(*context, GetDirtyMapList).WillByDefault(ReturnRef(dummy));

    MapUpdateLogWriteContext mapUpdatedLogWriteContext(context, &notifier, &sequencer);

    // When, Then
    bool retResult = true;
    {
        InSequence s;

        EXPECT_CALL(sequencer, GetCallbackExecutionApproval);
        EXPECT_CALL(*clientCallback, Execute).WillOnce(Return(retResult));
        EXPECT_CALL(sequencer, NotifyCallbackCompleted);
        EXPECT_CALL(notifier, NotifyLogFilled);
    }
    mapUpdatedLogWriteContext.IoDone();

    delete context;
}

} // namespace pos
