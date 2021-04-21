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
#include "src/include/meta_const.h"
#include "src/include/pos_event_id.hpp"
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
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_request_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/io/general_io/vsa_range_maker_mock.h"
#include "test/unit-tests/journal_service/journal_service_mock.h"
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
TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_Constructor_FourArgument_Stack)
{
    // Given
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    MockIBlockAllocator iBlockAllocator;
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(mockVolumeIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        mockVolumeIo, mockCallback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(
        mockVolumeIo, mockCallback, []() -> bool { return false; }, &mockIVSAMap, &mockJournalService,
        &mockEventScheduler, mockBlockMapUpdateCompletion);

    // When: Create New BlockMapUpdateReqeust Object with 4 argument
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIo, mockCallback,
        mockBlockMapUpdateEvent, &mockEventScheduler);

    // Then: Do nothing
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_Constructor_FourArgument_Heap)
{
    // Given
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    MockIBlockAllocator iBlockAllocator;
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(mockVolumeIo);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        mockVolumeIo, mockCallback, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);
    EventSmartPtr mockBlockMapUpdateEvent = std::make_shared<MockBlockMapUpdate>(
        mockVolumeIo, mockCallback, []() -> bool { return false; }, &mockIVSAMap, &mockJournalService,
        &mockEventScheduler, mockBlockMapUpdateCompletion);

    // When: Create New BlockMapUpdateReqeust Object with 4 argument
    BlockMapUpdateRequest* blockMapUpdateRequest = new BlockMapUpdateRequest(mockVolumeIo, mockCallback,
        mockBlockMapUpdateEvent, &mockEventScheduler);

    // Then: Do nothing
    delete blockMapUpdateRequest;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_NormalCase)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, ""));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    NiceMock<MockStripe> mockStripe;
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    MockIBlockAllocator iBlockAllocator;
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(mockVolumeIoPtr);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        mockVolumeIoPtr, mockCallbackPtr, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);

    NiceMock<MockBlockMapUpdate>* mockBlockMapUpdate = new NiceMock<MockBlockMapUpdate>(
        mockVolumeIoPtr, mockCallbackPtr, []() -> bool { return false; }, &mockIVSAMap, &mockJournalService,
        &mockEventScheduler, mockBlockMapUpdateCompletion);
    EventSmartPtr mockBlockMapUpdateEvent(mockBlockMapUpdate);

    // When: BlockMapUpdate act as normal
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        mockBlockMapUpdateEvent, &mockEventScheduler);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));

    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
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

    NiceMock<MockStripe> mockStripe;
    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockJournalService> mockJournalService;
    NiceMock<MockEventScheduler> mockEventScheduler;
    MockIBlockAllocator iBlockAllocator;
    MockWriteCompletion* mockWriteCompletion = new NiceMock<MockWriteCompletion>(mockVolumeIoPtr);
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockVsaRangeMaker>* mockVsaRangeMaker(new NiceMock<MockVsaRangeMaker>(0, 0, 0, false, &mockIVSAMap));
    MockBlockMapUpdateCompletion* mockBlockMapUpdateCompletion = new NiceMock<MockBlockMapUpdateCompletion>(
        mockVolumeIoPtr, mockCallbackPtr, []() -> bool { return false; }, &mockIVSAMap, &mockEventScheduler, mockWriteCompletion,
        &iBlockAllocator, &mockIWBStripeAllocator, mockVsaRangeMaker);

    NiceMock<MockBlockMapUpdate>* mockBlockMapUpdate = new NiceMock<MockBlockMapUpdate>(
        mockVolumeIoPtr, mockCallbackPtr, []() -> bool { return false; }, &mockIVSAMap, &mockJournalService,
        &mockEventScheduler, mockBlockMapUpdateCompletion);
    EventSmartPtr mockBlockMapUpdateEvent(mockBlockMapUpdate);

    // When: BlockMapUpdate Event is not nullptr, but failed to execute.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        mockBlockMapUpdateEvent, &mockEventScheduler);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));
    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());

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
    NiceMock<MockEventScheduler> mockEventScheduler;
    NiceMock<MockStripe> mockStripe;
    NiceMock<MockIVSAMap> mockIVSAMap;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;

    // When: BlockMapUpdate event is nullptr.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        nullptr, &mockEventScheduler);

    bool actual, expected;
    ON_CALL(*mockVolumeIo, GetLsidEntry()).WillByDefault(ReturnRef(addr));
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(*mockVolumeIo, GetSectorRba()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, IsGc()).WillByDefault(Return(false));
    ON_CALL(mockStripe, UpdateReverseMap(_, _, _)).WillByDefault(Return());
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest throws exception and return true.
    expected = true;
    ASSERT_EQ(actual, expected);
}

} // namespace pos
