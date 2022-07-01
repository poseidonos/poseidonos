#include "src/gc/gc_flush_completion.h"

#include <gtest/gtest.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

#include <test/unit-tests/allocator/stripe/stripe_mock.h>
#include <test/unit-tests/io/general_io/rba_state_manager_mock.h>
#include <test/unit-tests/gc/gc_map_update_request_mock.h>

#include "test/unit-tests/resource_manager/memory_manager_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
namespace pos
{

class GcFlushCompletionTestFixture : public ::testing::Test
{
public:
    GcFlushCompletionTestFixture(void)
    : gcFlushCompletion(nullptr),
      array(nullptr),
      gcStripeManager(nullptr),
      volumeEventPublisher(nullptr)
    {
    }

    virtual ~GcFlushCompletionTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        testVolumeId = 1;
        arrayName = "POSArray";

        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        memoryManager = new MockMemoryManager();
        EXPECT_CALL(*memoryManager, CreateBufferPool).WillRepeatedly(Return(nullptr));
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, volumeEventPublisher, memoryManager);

        stripe = new NiceMock<MockStripe>();
        stripeSmartPtr = StripeSmartPtr(stripe);
        rbaStateManager = new NiceMock<MockRBAStateManager>(arrayName, 0);

        inputEvent = std::make_shared<MockGcMapUpdateRequest>(StripeSmartPtr(new NiceMock<MockStripe>()), nullptr, nullptr, array, nullptr);
    }

    virtual void
    TearDown(void)
    {
        delete gcFlushCompletion;
        delete array;
        delete gcStripeManager;
        delete volumeEventPublisher;
        delete rbaStateManager;
        delete memoryManager;

        inputEvent = nullptr;
        stripeSmartPtr = nullptr;
    }

protected:
    GcFlushCompletion* gcFlushCompletion;

    uint32_t testVolumeId;
    std::string arrayName;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockStripe>* stripe;
    StripeSmartPtr stripeSmartPtr;
    NiceMock<MockRBAStateManager>* rbaStateManager;
    MockMemoryManager* memoryManager;

    GcWriteBuffer* dataBuffer;
    EventSmartPtr inputEvent;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 32,
    .totalStripes = 3200,
    .totalSegments = 100,
    };
};

TEST_F(GcFlushCompletionTestFixture, Execute_testIfgcFlushCompletionWhenAcquireRbaOwnershipFail)
{
    // given create gc flush completion and can not acquire rba ownership
    dataBuffer = new GcWriteBuffer();
    gcFlushCompletion = new GcFlushCompletion(stripeSmartPtr, arrayName, gcStripeManager, dataBuffer,
                                inputEvent,
                                rbaStateManager,
                                array);
    EXPECT_CALL(*gcStripeManager, ReturnBuffer(dataBuffer)).Times(1);

    for (uint32_t index = 1 ; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillOnce(Return(revMapEntry));
    }
    std::pair<uint32_t, uint32_t> revMapEntry = {0, testVolumeId};
    EXPECT_CALL(*stripe, GetReverseMapEntry(0)).WillOnce(Return(revMapEntry)).WillOnce(Return(revMapEntry));
    EXPECT_CALL(*rbaStateManager, AcquireOwnershipRbaList(testVolumeId, _)).WillOnce(Return(false));
    // when gc flush completion execute
    // then return false
    EXPECT_TRUE(gcFlushCompletion->Execute() == false);

    delete dataBuffer;
}

TEST_F(GcFlushCompletionTestFixture, Execute_testgcFlushExecuteWhenAcquireOwnershipWithRbaListSuccess)
{
    // given create gc flush completion and acquire rba ownership
    dataBuffer = nullptr;
    gcFlushCompletion = new GcFlushCompletion(stripeSmartPtr, arrayName, gcStripeManager, dataBuffer,
                                inputEvent,
                                rbaStateManager,
                                array);

    for (uint32_t index = 1 ; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillOnce(Return(revMapEntry));
    }
    std::pair<uint32_t, uint32_t> revMapEntry = {0, testVolumeId};
    EXPECT_CALL(*stripe, GetReverseMapEntry(0)).WillOnce(Return(revMapEntry)).WillOnce(Return(revMapEntry));
    EXPECT_CALL(*rbaStateManager, AcquireOwnershipRbaList(testVolumeId, _)).WillOnce(Return(true));
    EXPECT_CALL(*stripe, Flush(inputEvent)).Times(1);
    // when gc flush completion
    // then return true and stripe flushed
    EXPECT_TRUE(gcFlushCompletion->Execute() == true);
}
} // namespace pos
