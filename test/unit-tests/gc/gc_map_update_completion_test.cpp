#include "src/gc/gc_map_update_completion.h"

#include <gtest/gtest.h>
#include <test/unit-tests/allocator/i_block_allocator_mock.h>
#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/allocator/stripe/stripe_mock.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/cpu_affinity/affinity_manager_mock.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>
#include <test/unit-tests/gc/gc_map_update_request_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/io/general_io/rba_state_manager_mock.h>
#include <test/unit-tests/mapper/i_stripemap_mock.h>
#include <test/unit-tests/mapper/i_vsamap_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/utils/mock_builder.h>
#include <test/unit-tests/volume/i_volume_io_manager_mock.h>

#include "test/unit-tests/resource_manager/memory_manager_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
namespace pos
{
class GcMapUpdateCompletionTestFixture : public ::testing::Test
{
public:
    GcMapUpdateCompletionTestFixture(void)
    : gcMapUpdateCompletion(nullptr),
      array(nullptr),
      gcStripeManager(nullptr),
      volumeEventPublisher(nullptr)
    {
    }

    virtual ~GcMapUpdateCompletionTestFixture(void)
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
        rbaStateManager = new NiceMock<MockRBAStateManager>(arrayName, 0);

        stripeMap = new NiceMock<MockIStripeMap>;
        vsaMap = new NiceMock<MockIVSAMap>;
        eventScheduler = new NiceMock<MockEventScheduler>;

        contextManager = new NiceMock<MockIContextManager>;
        blockAllocator = new NiceMock<MockIBlockAllocator>;
        volumeManager = new NiceMock<MockIVolumeIoManager>;
    }

    virtual void
    TearDown(void)
    {
        delete gcMapUpdateCompletion;
        delete array;
        delete gcStripeManager;
        delete volumeEventPublisher;
        delete rbaStateManager;
        delete vsaMap;
        delete stripeMap;
        delete eventScheduler;
        delete blockAllocator;
        delete contextManager;
        delete volumeManager;
        delete memoryManager;
    }

protected:
    GcMapUpdateCompletion* gcMapUpdateCompletion;

    uint32_t testVolumeId;
    std::string arrayName;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockStripe>* stripe;
    NiceMock<MockRBAStateManager>* rbaStateManager;

    GcWriteBuffer* dataBuffer;

    NiceMock<MockIStripeMap>* stripeMap;
    NiceMock<MockIVSAMap>* vsaMap;
    NiceMock<MockEventScheduler>* eventScheduler;
    NiceMock<MockIContextManager>* contextManager;
    NiceMock<MockIBlockAllocator>* blockAllocator;

    NiceMock<MockIVolumeIoManager>* volumeManager;
    MockMemoryManager* memoryManager;

    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t> invalidSegCnt;

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

TEST_F(GcMapUpdateCompletionTestFixture, Execute_testGcMapCompletionReleaseRbaOwnershipAndDecreasePendingIo)
{
    gcMapUpdateCompletion = new GcMapUpdateCompletion(StripeSmartPtr(stripe), arrayName, stripeMap, eventScheduler,
        gcStripeManager, array, rbaStateManager, volumeManager);

    // given rba list
    std::list<RbaAndSize> rbaList;
    for (uint32_t index = 0; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillRepeatedly(Return(revMapEntry));
        RbaAndSize rbaAndSize = {index * VolumeIo::UNITS_PER_BLOCK, BLOCK_SIZE};
        rbaList.push_back(rbaAndSize);
    }
    // when gc map update request
    // then release rba ownership and decrease pending io and set finished
    EXPECT_CALL(*rbaStateManager, ReleaseOwnershipRbaList(testVolumeId, rbaList)).Times(1);
    EXPECT_CALL(*volumeManager, DecreasePendingIOCount(testVolumeId, _, _)).Times(1);
    EXPECT_CALL(*gcStripeManager, SetFinished).Times(1);
    EXPECT_TRUE(gcMapUpdateCompletion->Execute() == true);
}

} // namespace pos
