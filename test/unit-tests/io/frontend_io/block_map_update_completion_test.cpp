#include "src/io/frontend_io/block_map_update_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_NineArgument_Stack)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, 0));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr mockWriteCompletionEvent = std::make_shared<MockWriteCompletion>(volumeIo);

    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(
        volumeIo, callback, false, &mockEventScheduler, mockWriteCompletionEvent);
    // Then : Do nothing
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_NineArgument_Heap)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, 0));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    CallbackSmartPtr mockWriteCompletionEvent = std::make_shared<MockWriteCompletion>(volumeIo);

    // When
    BlockMapUpdateCompletion* blockMapUpdateCompletion = new BlockMapUpdateCompletion(
        volumeIo, callback, false, &mockEventScheduler, mockWriteCompletionEvent);

    // Then : Do nothing
    delete blockMapUpdateCompletion;
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_Execute)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockCallback* mockCallback = new NiceMock<MockCallback>(true, 0);
    CallbackSmartPtr callback(mockCallback);
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(volumeIo);
    CallbackSmartPtr mockWriteCompletionEvent(mockWriteCompletion);
    NiceMock<MockStripe> mockStripe;

    ON_CALL(*mockWriteCompletion, _DoSpecificJob()).WillByDefault(Return(true));
    ON_CALL(*mockCallback, _RecordCallerCompletionAndCheckOkToCall(_, _, _)).WillByDefault(Return(false));

    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(
        volumeIo, callback, false,
        &mockEventScheduler, mockWriteCompletionEvent);

    // Then : Execute
    bool actual = blockMapUpdateCompletion.Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_ExecuteFail)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, 0);
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockCallback* mockCallback = new NiceMock<MockCallback>(true, 0);
    CallbackSmartPtr callback(mockCallback);
    MockWriteCompletion* mockWriteCompletionFail = new NiceMock<MockWriteCompletion>(volumeIo);
    CallbackSmartPtr mockWriteCompletionEvent(mockWriteCompletionFail);
    NiceMock<MockStripe> mockStripe;
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};

    ON_CALL(*mockWriteCompletionFail, _DoSpecificJob()).WillByDefault(Return(false));
    ON_CALL(*mockCallback, _RecordCallerCompletionAndCheckOkToCall(_, _, _)).WillByDefault(Return(false));

    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());

    // When : Event (write completion) failed
    ON_CALL(*mockWriteCompletionFail, _DoSpecificJob()).WillByDefault(Return(false));

    BlockMapUpdateCompletion blockMapUpdateCompletionFail(
        volumeIo, callback, false,
        &mockEventScheduler, mockWriteCompletionEvent);

    // Then : Execute
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);
    bool actual = blockMapUpdateCompletionFail.Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
}

} // namespace pos
