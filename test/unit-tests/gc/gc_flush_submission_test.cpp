#include "src/gc/gc_flush_submission.h"

#include <gtest/gtest.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

#include <test/unit-tests/allocator/stripe/stripe_mock.h>
#include <test/unit-tests/io/general_io/rba_state_manager_mock.h>
#include <test/unit-tests/gc/gc_map_update_request_mock.h>
#include <test/unit-tests/allocator/i_block_allocator_mock.h>
#include <test/unit-tests/io_submit_interface/i_io_submit_handler_mock.h>
#include <test/unit-tests/gc/flow_control/flow_control_mock.h>
#include <test/unit-tests/gc/gc_flush_completion_mock.h>

#include "test/unit-tests/resource_manager/memory_manager_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;

namespace pos
{

class GcFlushSubmissionTestFixture : public ::testing::Test
{
public:
    GcFlushSubmissionTestFixture(void)
    : gcFlushSubmission(nullptr),
      array(nullptr),
      gcStripeManager(nullptr),
      volumeEventPublisher(nullptr)
    {
    }

    virtual ~GcFlushSubmissionTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        testVolumeId = 1;
        arrayName = "POSArray";
        arrayIndex = 0;

        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        memoryManager = new MockMemoryManager();
        EXPECT_CALL(*memoryManager, CreateBufferPool).WillRepeatedly(Return(nullptr));
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, volumeEventPublisher, memoryManager);

        stripe = new NiceMock<MockStripe>();
        rbaStateManager = new NiceMock<MockRBAStateManager>(arrayName, 0);

        inputEvent = std::make_shared<MockGcMapUpdateRequest>(StripeSmartPtr(new NiceMock<MockStripe>()), nullptr, nullptr, array, nullptr);

        blockAllocator = new NiceMock<MockIBlockAllocator>();
        ioSubmitHandler = new NiceMock<MockIIOSubmitHandler>();
        flowControl = new NiceMock<MockFlowControl>(array, nullptr,
                        nullptr, nullptr, nullptr, nullptr);
    }

    virtual void
    TearDown(void)
    {
        delete gcFlushSubmission;
        delete array;
        delete gcStripeManager;
        delete volumeEventPublisher;
        delete rbaStateManager;
        delete blockAllocator;
        delete ioSubmitHandler;
        delete flowControl;
        delete memoryManager;

        inputEvent = nullptr;
    }

protected:
    GcFlushSubmission* gcFlushSubmission;

    uint32_t testVolumeId;
    std::string arrayName;
    int arrayIndex;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockStripe>* stripe;
    NiceMock<MockRBAStateManager>* rbaStateManager;

    GcWriteBuffer* dataBuffer;
    EventSmartPtr inputEvent;
    std::vector<BlkInfo>* blkInfoList;

    CallbackSmartPtr callback;
    NiceMock<MockIBlockAllocator>* blockAllocator;
    NiceMock<MockIIOSubmitHandler>* ioSubmitHandler;
    NiceMock<MockFlowControl>* flowControl;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 32,
    .totalStripes = 3200,
    .totalSegments = 100,
    };
    MockMemoryManager* memoryManager;
};

TEST_F(GcFlushSubmissionTestFixture, Execute_testIfgcFlushSubmissionExecuteWhenGetTokenFail)
{
    // given gc flush submission and get token fail
    gcFlushSubmission = new GcFlushSubmission(arrayName, blkInfoList, testVolumeId,
                        dataBuffer, gcStripeManager, callback, blockAllocator,
                        ioSubmitHandler, flowControl, array);
    EXPECT_CALL(*flowControl, GetToken(_, _)).WillOnce(Return(-1));

    // when Execute
    // then return false
    EXPECT_TRUE(gcFlushSubmission->Execute() == false);

    delete stripe;
}

TEST_F(GcFlushSubmissionTestFixture, Execute_testIfgcFlushSubmissionExecuteWhenAllocateGcStripeFail)
{
    // given gc flush submission and allocate gc stripe fail
    gcFlushSubmission = new GcFlushSubmission(arrayName, blkInfoList, testVolumeId,
                        dataBuffer, gcStripeManager, callback, blockAllocator,
                        ioSubmitHandler, flowControl, array);
    EXPECT_CALL(*flowControl, GetToken(_, _)).WillOnce(Return(partitionLogicalSize.blksPerStripe));
    EXPECT_CALL(*blockAllocator, AllocateGcDestStripe(testVolumeId)).WillOnce(nullptr);
    EXPECT_CALL(*flowControl, ReturnToken(_, partitionLogicalSize.blksPerStripe)).Times(1);

    // when Execute
    // then return false
    EXPECT_TRUE(gcFlushSubmission->Execute() == false);

    delete stripe;
}

TEST_F(GcFlushSubmissionTestFixture, Execute_testIfExecuteWhenGetTokenAndAllocateGcStripeSuccess)
{
    // given token, gc stripe, copied block info
    StripeId vsid = 100;
    blkInfoList = new std::vector<BlkInfo>();
    for (uint32_t index = 0; index < partitionLogicalSize.blksPerStripe; index++)
    {
        BlkInfo blkInfo = {.rba = index, .volID = testVolumeId,
                        .vsa = {.stripeId = vsid, .offset = index}};
        blkInfoList->push_back(blkInfo);

        EXPECT_CALL(*stripe, UpdateVictimVsa(index, blkInfo.vsa)).Times(1);
    }

    dataBuffer = new GcWriteBuffer;
    void* buffer = (void*)0x20000000;
    dataBuffer->push_back(buffer);
    StripeSmartPtr testStripe(new NiceMock<MockStripe>());
    callback = std::make_shared<MockGcFlushCompletion>(testStripe, arrayName, gcStripeManager, dataBuffer, nullptr, nullptr, array);

    gcFlushSubmission = new GcFlushSubmission(arrayName, blkInfoList, testVolumeId,
                        dataBuffer, gcStripeManager, callback, blockAllocator,
                        ioSubmitHandler, flowControl, array);
    EXPECT_CALL(*flowControl, GetToken(_, _)).WillOnce(Return(partitionLogicalSize.blksPerStripe));
    EXPECT_CALL(*blockAllocator, AllocateGcDestStripe(testVolumeId)).WillOnce(Return(stripe));

    EXPECT_CALL(*ioSubmitHandler, SubmitAsyncIO(_, _, _, _, _, _, arrayIndex, _)).WillOnce(Return(IOSubmitHandlerStatus::SUCCESS));

    // when execute
    // then submit async io for gc stripe flush
    EXPECT_TRUE(gcFlushSubmission->Execute() == true);

    dataBuffer->clear();
    delete dataBuffer;
    callback = nullptr;
}

} // namespace pos
