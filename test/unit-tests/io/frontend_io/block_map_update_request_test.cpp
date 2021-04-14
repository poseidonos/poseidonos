#include "src/io/frontend_io/block_map_update_request.h"

#include <gtest/gtest.h>

#include <functional>

#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/array/array.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/address_type.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/include/meta_const.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/io/general_io/vsa_range_maker.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_interface_container.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/allocator/wb_stripe_manager/stripe_mock.h"
#include "test/unit-tests/allocator_service/allocator_service_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_request_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_Constructor_SevenArgument_Stack)
{
    // Given
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 0};
    NiceMock<MockAllocatorService> allocatorService;
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(mockVolumeIo, mockCallback);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: Create New BlockMapUpdateReqeust Object with 7 argument
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIo, mockCallback,
        &allocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    // Then: Do nothing
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_Constructor_SevenArgument_Heap)
{
    // Given
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));
    bool retryNeeded = false;
    NiceMock<MockAllocatorService> allocatorService;
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(mockVolumeIo, mockCallback);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: Create New BlockMapUpdateReqeust Object with 7 argument
    BlockMapUpdateRequest* blockMapUpdateRequest = new BlockMapUpdateRequest(mockVolumeIo, mockCallback,
        &allocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    // Then: Do nothing
    delete blockMapUpdateRequest;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_NeedRetry)
{
    //Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(mockVolumeIoPtr, mockCallbackPtr);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: VsaRangeMaker Need Retry
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));

    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockVsaRangeMaker, CheckRetry()).WillByDefault(Return(true));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdate Reqeust should fail
    expected = false;
    ASSERT_EQ(actual, expected);
    delete mockVolumeIo;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_GetStripeFail)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(mockVolumeIoPtr, mockCallbackPtr);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: GetStripe Failed
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));

    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(nullptr));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest should fail.
    expected = true;
    ASSERT_EQ(actual, expected);
    delete mockVolumeIo;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_NormalCase)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockBlockMapUpdate>* mockBlockMapUpdate = new NiceMock<MockBlockMapUpdate>(mockVolumeIoPtr, mockCallbackPtr);
    EventSmartPtr mockBlockMapUpdateEvent(mockBlockMapUpdate);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: BlockMapUpdate act as normal
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));

    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockVsaRangeMaker, CheckRetry()).WillByDefault(Return(false));
    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(1));
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 0};
    ON_CALL(*mockVsaRangeMaker, GetVsaRange(_)).WillByDefault(ReturnRef(vsaRange));
    ON_CALL(*mockBlockMapUpdate, Execute()).WillByDefault(Return(true));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest should success.
    expected = true;
    ASSERT_EQ(actual, expected);
    delete mockVolumeIo;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_BlockMapUpdateExecuteFail)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockBlockMapUpdate>* mockBlockMapUpdate = new NiceMock<MockBlockMapUpdate>(mockVolumeIoPtr, mockCallbackPtr);
    EventSmartPtr mockBlockMapUpdateEvent(mockBlockMapUpdate);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: BlockMapUpdate Event is not nullptr, but failed to execute.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));
    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockVsaRangeMaker, CheckRetry()).WillByDefault(Return(false));
    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(1));
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 0};
    ON_CALL(*mockVsaRangeMaker, GetVsaRange(_)).WillByDefault(ReturnRef(vsaRange));
    ON_CALL(*mockBlockMapUpdate, Execute()).WillByDefault(Return(false));
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest throws exception and return true.
    expected = true;
    ASSERT_EQ(actual, expected);
    delete mockVolumeIo;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_BlockMapUpdateEventNull)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));

    // When: BlockMapUpdate event is nullptr.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, nullptr, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));
    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockVsaRangeMaker, CheckRetry()).WillByDefault(Return(false));
    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(1));
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 0};
    ON_CALL(*mockVsaRangeMaker, GetVsaRange(_)).WillByDefault(ReturnRef(vsaRange));
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest throws exception and return true.
    expected = true;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_OldDataTrueWithGcOn)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockAllocatorService> mockAllocatorService;
    NiceMock<MockBlockMapUpdate>* mockBlockMapUpdate = new NiceMock<MockBlockMapUpdate>(mockVolumeIoPtr, mockCallbackPtr);
    EventSmartPtr mockBlockMapUpdateEvent(mockBlockMapUpdate);
    WriteCompletionFunc mockWriteCompletionFunc = [](VolumeIoSmartPtr mockVolumeIoPtr, CallbackSmartPtr mockCallbackPtr) { return true; };
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, true, &mockIVSAMap));

    // When: GC is on & oldData is true.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        &mockAllocatorService, mockBlockMapUpdateEvent, mockWriteCompletionFunc, &mockEventScheduler, mockVsaRangeMaker);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(true));

    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockAllocatorService, GetIWBStripeAllocator(_)).WillByDefault(Return(&mockIWBStripeAllocator));
    ON_CALL(mockAllocatorService, GetIBlockAllocator(_)).WillByDefault(Return(&mockIBlockAllocator));
    ON_CALL(mockIWBStripeAllocator, GetStripe(_)).WillByDefault(Return(&mockStripe));
    ON_CALL(*mockVsaRangeMaker, CheckRetry()).WillByDefault(Return(false));
    ON_CALL(*mockVsaRangeMaker, GetCount()).WillByDefault(Return(1));
    VirtualBlks vsaRange = {.startVsa = vsa, .numBlks = 0};
    VirtualBlkAddr oldVsa = {.stripeId = 1, .offset = 0};
    ON_CALL(*mockVsaRangeMaker, GetVsaRange(_)).WillByDefault(ReturnRef(vsaRange));
    ON_CALL(*mockVolumeIo, GetOldVsa()).WillByDefault(ReturnRef(oldVsa));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest should success.
    expected = true;
    ASSERT_EQ(actual, expected);
    delete mockVolumeIo;
}
} // namespace pos
