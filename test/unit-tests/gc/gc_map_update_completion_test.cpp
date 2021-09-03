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
#include <test/unit-tests/journal_service/i_journal_writer_mock.h>
#include <test/unit-tests/journal_service/journal_service_mock.h>
#include <test/unit-tests/mapper/i_stripemap_mock.h>
#include <test/unit-tests/mapper/i_vsamap_mock.h>
#include <test/unit-tests/spdk_wrapper/free_buffer_pool_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/utils/mock_builder.h>
#include <test/unit-tests/volume/i_volume_manager_mock.h>

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
      affinityManager(nullptr),
      gcWriteBufferPool(nullptr),
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

        affinityManager = new NiceMock<MockAffinityManager>(BuildDefaultAffinityManagerMock());
        gcWriteBufferPool = new NiceMock<MockFreeBufferPool>(0, 0, affinityManager);
        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, gcWriteBufferPool, volumeEventPublisher);

        stripe = new NiceMock<MockStripe>();
        rbaStateManager = new NiceMock<MockRBAStateManager>(arrayName, 0);

        stripeMap = new NiceMock<MockIStripeMap>;
        vsaMap = new NiceMock<MockIVSAMap>;
        journal = new NiceMock<MockJournalService>;
        eventScheduler = new NiceMock<MockEventScheduler>;

        contextManager = new NiceMock<MockIContextManager>;
        blockAllocator = new NiceMock<MockIBlockAllocator>;
        volumeManager = new NiceMock<MockIVolumeManager>;
    }

    virtual void
    TearDown(void)
    {
        delete gcMapUpdateCompletion;
        delete array;
        delete affinityManager;
        delete gcStripeManager;
        delete volumeEventPublisher;
        delete rbaStateManager;
        delete vsaMap;
        delete journal;
        delete stripeMap;
        delete eventScheduler;
        delete blockAllocator;
        delete contextManager;
        delete volumeManager;
    }

protected:
    GcMapUpdateCompletion* gcMapUpdateCompletion;

    uint32_t testVolumeId;
    std::string arrayName;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockAffinityManager>* affinityManager;
    NiceMock<MockFreeBufferPool>* gcWriteBufferPool;
    NiceMock<MockStripe>* stripe;
    NiceMock<MockRBAStateManager>* rbaStateManager;

    GcWriteBuffer* dataBuffer;

    NiceMock<MockIStripeMap>* stripeMap;
    NiceMock<MockIVSAMap>* vsaMap;
    NiceMock<MockJournalService>* journal;
    NiceMock<MockEventScheduler>* eventScheduler;
    NiceMock<MockIContextManager>* contextManager;
    NiceMock<MockIBlockAllocator>* blockAllocator;

    NiceMock<MockIVolumeManager>* volumeManager;

    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t> invalidSegCnt;

    PartitionLogicalSize partitionLogicalSize = {
        .minWriteBlkCnt = 0 /*not interesting*/,
        .blksPerChunk = 64,
        .blksPerStripe = 2048,
        .chunksPerStripe = 32,
        .stripesPerSegment = 1024,
        .totalStripes = 32,
        .totalSegments = 32768,
    };
};

TEST_F(GcMapUpdateCompletionTestFixture, Execute_testGcMapCompletionReleaseRbaOwnershipAndDecreasePendingIo)
{
    gcMapUpdateCompletion = new GcMapUpdateCompletion(stripe, arrayName, stripeMap, eventScheduler,
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
