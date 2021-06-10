#include "src/io/frontend_io/block_map_update_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"

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
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIBlockAllocator> iBlockAllocator;
    CallbackSmartPtr mockWriteCompletionEvent = std::make_shared<MockWriteCompletion>(volumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &vsaMap));

    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(
        volumeIo, callback, false, &vsaMap, &mockEventScheduler,
        mockWriteCompletionEvent, &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    // Then : Do nothing
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_NineArgument_Heap)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    NiceMock<MockIBlockAllocator> iBlockAllocator;
    CallbackSmartPtr mockWriteCompletionEvent = std::make_shared<MockWriteCompletion>(volumeIo);
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &vsaMap));

    // When
    BlockMapUpdateCompletion* blockMapUpdateCompletion = new BlockMapUpdateCompletion(
        volumeIo, callback, false, &vsaMap, &mockEventScheduler,
        mockWriteCompletionEvent, &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    // Then : Do nothing
    delete blockMapUpdateCompletion;
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_Execute)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockCallback* mockCallback = new NiceMock<MockCallback>(true, 0);
    CallbackSmartPtr callback(mockCallback);
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(volumeIo);
    CallbackSmartPtr mockWriteCompletionEvent(mockWriteCompletion);
    NiceMock<MockIBlockAllocator> iBlockAllocator;
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    MockVsaRangeMaker* mockVsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &vsaMap);
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};

    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    ON_CALL(vsaMap, SetVSAs(_, _, _)).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockWriteCompletion, _DoSpecificJob()).WillByDefault(Return(true));
    ON_CALL(*mockCallback, _RecordCallerCompletionAndCheckOkToCall(_, _, _)).WillByDefault(Return(false));
    ON_CALL(iBlockAllocator, ValidateBlks(_)).WillByDefault(Return());
    EXPECT_CALL(iBlockAllocator, ValidateBlks(_)).Times(1);

    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(
        volumeIo, callback, false,
        &vsaMap, &mockEventScheduler, mockWriteCompletionEvent, &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);

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
    MockVolumeIo* mockVolumeIo = new NiceMock<MockVolumeIo>((void*)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    MockCallback* mockCallback = new NiceMock<MockCallback>(true, 0);
    CallbackSmartPtr callback(mockCallback);
    MockWriteCompletion* mockWriteCompletionFail = new NiceMock<MockWriteCompletion>(volumeIo);
    CallbackSmartPtr mockWriteCompletionEvent(mockWriteCompletionFail);
    NiceMock<MockIBlockAllocator> iBlockAllocator;
    NiceMock<MockIVSAMap> vsaMap;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    MockVsaRangeMaker* mockVsaRangeMaker = new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &vsaMap);
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};

    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    ON_CALL(vsaMap, SetVSAs(_, _, _)).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockWriteCompletionFail, _DoSpecificJob()).WillByDefault(Return(false));
    ON_CALL(*mockCallback, _RecordCallerCompletionAndCheckOkToCall(_, _, _)).WillByDefault(Return(false));
    ON_CALL(iBlockAllocator, ValidateBlks(_)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    EXPECT_CALL(iBlockAllocator, ValidateBlks(_)).Times(1);

    // When : Event (write completion) failed
    ON_CALL(*mockWriteCompletionFail, _DoSpecificJob()).WillByDefault(Return(false));

    BlockMapUpdateCompletion blockMapUpdateCompletionFail(
        volumeIo, callback, false,
        &vsaMap, &mockEventScheduler, mockWriteCompletionEvent, &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);

    // Then : Execute
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);
    bool actual = blockMapUpdateCompletionFail.Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);
}

} // namespace pos
