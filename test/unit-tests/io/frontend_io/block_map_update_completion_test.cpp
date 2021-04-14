#include "src/io/frontend_io/block_map_update_completion.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"
#include "test/unit-tests/mapper_service/mapper_service_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/io/frontend_io/block_map_update_completion_mock.h"
#include "test/unit-tests/io/frontend_io/write_completion_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "src/event_scheduler/callback.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"

using namespace pos;
using namespace std;
using ::testing::AtLeast;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_Constructor)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    MockIBlockAllocator iBlockAllocator;
    MockWriteCompletion *mockWriteCompletion 
        = new NiceMock<MockWriteCompletion>(volumeIo);

    NiceMock<MockIVSAMap> vsaMap;

    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(volumeIo, callback, []()->bool{return false;}, 
        &vsaMap, &mockEventScheduler, mockWriteCompletion, &iBlockAllocator);
    // Then : Do nothing
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_TwoArgument)
{
     // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    VolumeIoSmartPtr volumeIo(new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, ""));
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    MockWriteCompletion *mockWriteCompletion 
        = new NiceMock<MockWriteCompletion>(volumeIo);
    // When
    BlockMapUpdateCompletion blockMapUpdateCompletion(volumeIo, callback);
}

TEST(BlockMapUpdateCompletion, BlockMapUpdateCompletion_Execute)
{
    // Given
    const uint32_t unitCount = 8;

    NiceMock<MockEventScheduler> mockEventScheduler;
    MockVolumeIo *mockVolumeIo = new NiceMock<MockVolumeIo>((void *)0xff00, unitCount, "");
    VolumeIoSmartPtr volumeIo(mockVolumeIo);
    CallbackSmartPtr callback(new NiceMock<MockCallback>(true));
    MockWriteCompletion *mockWriteCompletion 
        = new NiceMock<MockWriteCompletion>(volumeIo);
    MockIBlockAllocator iBlockAllocator;

    NiceMock<MockIVSAMap> vsaMap;
    VirtualBlkAddr vsa = {.stripeId = 0, .offset = 0};
    ON_CALL(*mockVolumeIo, GetVsa()).WillByDefault(ReturnRef(vsa));
    ON_CALL(vsaMap, SetVSAsInternal(_,_,_)).WillByDefault(Return(0));
    ON_CALL(vsaMap, SetVSAs(_,_,_)).WillByDefault(Return(0));
    ON_CALL(*mockVolumeIo, GetVolumeId()).WillByDefault(Return(0));
    ON_CALL(*mockWriteCompletion, _DoSpecificJob()).WillByDefault(Return(true));
    EXPECT_CALL(iBlockAllocator, ValidateBlks(_)).Times(1);
    // When
    
    BlockMapUpdateCompletion blockMapUpdateCompletion(volumeIo, callback, []()->bool{return false;}, 
        &vsaMap, &mockEventScheduler, mockWriteCompletion, &iBlockAllocator);
    // Then : Execute
    bool actual = blockMapUpdateCompletion.Execute();
    bool expected = true;
    ASSERT_EQ(actual, expected);

    MockWriteCompletion *mockWriteCompletionFail 
        = new NiceMock<MockWriteCompletion>(volumeIo);

    // When : Event (write completion) failed
    ON_CALL(*mockWriteCompletionFail, _DoSpecificJob()).WillByDefault(Return(false));
    EXPECT_CALL(iBlockAllocator, ValidateBlks(_)).Times(1);
    
    BlockMapUpdateCompletion blockMapUpdateCompletionFail(volumeIo, callback, []()->bool{return false;}, 
        &vsaMap, &mockEventScheduler, mockWriteCompletionFail, &iBlockAllocator);

    // Then : Execute
    EXPECT_CALL(mockEventScheduler, EnqueueEvent(_)).Times(1);
    actual = blockMapUpdateCompletionFail.Execute();
    expected = true;
    ASSERT_EQ(actual, expected);
}

} // namespace pos
