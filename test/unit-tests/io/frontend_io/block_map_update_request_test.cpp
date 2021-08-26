#include "src/io/frontend_io/block_map_update_request.h"

#include <gtest/gtest.h>

#include <functional>

#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/io_completer.h"
#include "src/include/address_type.h"
#include "src/include/branch_prediction.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/rba_state_manager.h"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/io/general_io/rba_state_manager_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/unit-tests/meta_service/i_meta_updater_mock.h"

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
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));

    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(mockVolumeIo);
    CallbackSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        mockVolumeIo, mockCallback, false, &mockEventScheduler, mockWriteCompletion);

    // When: Create New BlockMapUpdateReqeust Object with 5 argument
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIo, mockCallback,
        mockBlockMapUpdateCompletionEvent, &mockMetaUpdater, &mockEventScheduler, false);

    // Then: Do nothing
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_Constructor_FourArgument_Heap)
{
    // Given
    VolumeIoSmartPtr mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    CallbackSmartPtr mockCallback(new NiceMock<MockCallback>(true, 0));

    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(mockVolumeIo);
    CallbackSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        mockVolumeIo, mockCallback, false, &mockEventScheduler, mockWriteCompletion);

    // When: Create New BlockMapUpdateReqeust Object with 5 argument
    BlockMapUpdateRequest* blockMapUpdateRequest = new BlockMapUpdateRequest(mockVolumeIo, mockCallback,
        mockBlockMapUpdateCompletionEvent, &mockMetaUpdater, &mockEventScheduler, false);

    // Then: Do nothing
    delete blockMapUpdateRequest;
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_NormalCase)
{
    // Given
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(mockVolumeIoPtr);
    CallbackSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        mockVolumeIoPtr, mockCallbackPtr, false, &mockEventScheduler, mockWriteCompletion);

    // When: BlockMapUpdate act as normal
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        mockBlockMapUpdateCompletionEvent,  &mockMetaUpdater, &mockEventScheduler, false);

    bool actual, expected;
    ON_CALL(mockMetaUpdater, UpdateBlockMap(mockVolumeIoPtr, mockCallbackPtr)).WillByDefault(Return(0));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest should success.
    expected = true;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_GetErrorCount)
{
    // Given
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(mockVolumeIoPtr);
    CallbackSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        mockVolumeIoPtr, mockCallbackPtr, []() -> bool { return false; }, &mockEventScheduler, mockWriteCompletion);

    // When: BlockMapUpdate act as normal
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        mockBlockMapUpdateCompletionEvent, &mockMetaUpdater, &mockEventScheduler, false);

    bool actual, expected;
    ON_CALL(mockMetaUpdater, UpdateBlockMap(mockVolumeIoPtr, mockCallbackPtr)).WillByDefault(Return(0));

    blockMapUpdateRequest.InformError(IOErrorType::DEVICE_ERROR);
    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest should success.
    expected = true;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_BlockMapUpdateExecuteFail)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;
    CallbackSmartPtr mockWriteCompletion = std::make_shared<MockWriteCompletion>(mockVolumeIoPtr);
    CallbackSmartPtr mockBlockMapUpdateCompletionEvent = std::make_shared<MockBlockMapUpdateCompletion>(
        mockVolumeIoPtr, mockCallbackPtr, false, &mockEventScheduler, mockWriteCompletion);

    // When: BlockMapUpdate Event is not nullptr, but failed to execute.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        mockBlockMapUpdateCompletionEvent, &mockMetaUpdater, &mockEventScheduler, false);

    bool actual, expected;
    ON_CALL(mockMetaUpdater, UpdateBlockMap(mockVolumeIoPtr, _)).WillByDefault(Return(-1));

    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest return false
    expected = false;
    ASSERT_EQ(actual, expected);
}

TEST(BlockMapUpdateRequest, BlockMapUpdateRequest_DoSpecificJob_BlockMapUpdateCompletionEventNull)
{
    // Given
    StripeAddr addr = {.stripeLoc = IN_WRITE_BUFFER_AREA, .stripeId = 1};
    NiceMock<MockVolumeIo>* mockVolumeIo(new NiceMock<MockVolumeIo>(nullptr, 0, 0));
    NiceMock<MockCallback>* mockCallback(new NiceMock<MockCallback>(true, 0));

    VolumeIoSmartPtr mockVolumeIoPtr(mockVolumeIo);
    CallbackSmartPtr mockCallbackPtr(mockCallback);
    NiceMock<MockIMetaUpdater> mockMetaUpdater;
    NiceMock<MockEventScheduler> mockEventScheduler;

    // When: BlockMapUpdate event is nullptr.
    BlockMapUpdateRequest blockMapUpdateRequest(mockVolumeIoPtr, mockCallbackPtr,
        nullptr,  &mockMetaUpdater, &mockEventScheduler, false);

    bool actual, expected;
    actual = blockMapUpdateRequest.Execute();

    // Then: BlockMapUpdateRequest throws exception and return true.
    expected = true;
    ASSERT_EQ(actual, expected);
}
} // namespace pos
