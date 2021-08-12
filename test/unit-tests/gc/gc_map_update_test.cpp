#include "src/gc/gc_map_update.h"

#include <gtest/gtest.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/spdk_wrapper/free_buffer_pool_mock.h>
#include <test/unit-tests/cpu_affinity/affinity_manager_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

#include <test/unit-tests/allocator/stripe/stripe_mock.h>
#include <test/unit-tests/io/general_io/rba_state_manager_mock.h>
#include <test/unit-tests/gc/gc_map_update_request_mock.h>
#include <test/unit-tests/mapper/i_stripemap_mock.h>
#include <test/unit-tests/mapper/i_vsamap_mock.h>
#include <test/unit-tests/journal_service/journal_service_mock.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>
#include <test/unit-tests/journal_service/i_journal_writer_mock.h>
#include <test/unit-tests/allocator/i_block_allocator_mock.h>
#include <test/unit-tests/allocator/i_context_manager_mock.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
namespace pos
{

class GcMapUpdateTestFixture : public ::testing::Test
{
public:
    GcMapUpdateTestFixture(void)
    : gcMapUpdate(nullptr),
      array(nullptr),
      gcStripeManager(nullptr),
      affinityManager(nullptr),
      gcWriteBufferPool(nullptr),
      volumeEventPublisher(nullptr)
    {
    }

    virtual ~GcMapUpdateTestFixture(void)
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

        inputEvent = std::make_shared<MockGcMapUpdateRequest>(stripe, arrayName, gcStripeManager,
                        nullptr, nullptr, nullptr, nullptr, nullptr, array);

        stripeMap = new NiceMock<MockIStripeMap>;
        vsaMap = new NiceMock<MockIVSAMap>;
        journal = new NiceMock<MockJournalService>;
        eventScheduler = new NiceMock<MockEventScheduler>;

        contextManager = new NiceMock<MockIContextManager>;
        blockAllocator = new NiceMock<MockIBlockAllocator>;
    }

    virtual void
    TearDown(void)
    {
        delete gcMapUpdate;
        delete array;
        delete affinityManager;
        delete gcStripeManager;
        delete stripe;
        delete rbaStateManager;
        delete vsaMap;
        delete journal;
        delete stripeMap;
        delete eventScheduler;
        delete blockAllocator;
        delete contextManager;
        delete volumeEventPublisher;

        inputEvent = nullptr;
    }

protected:
    GcMapUpdate* gcMapUpdate;

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
    EventSmartPtr inputEvent;

    NiceMock<MockIStripeMap>* stripeMap;
    NiceMock<MockIVSAMap>* vsaMap;
    NiceMock<MockJournalService>* journal;
    NiceMock<MockEventScheduler>* eventScheduler;
    NiceMock<MockIContextManager>* contextManager;
    NiceMock<MockIBlockAllocator>* blockAllocator;


    GcStripeMapUpdateList mapUpdateInfoList;
    std::map<SegmentId, uint32_t > invalidSegCnt;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/*not interesting*/,
    .blksPerChunk = 64,
    .blksPerStripe = 2048,
    .chunksPerStripe = 32,
    .stripesPerSegment = 1024,
    .totalStripes = 32,
    .totalSegments = 32768,
    };
};

TEST_F(GcMapUpdateTestFixture, Execute_testGcMapUpdateWithValidMapAndInvalidSegBlks)
{
    SegmentId segId = 1;
    StripeId stripeId = 1024;
    uint32_t validMapCnt = 60;
    uint32_t invalidBlockCnt = 30;
    mapUpdateInfoList.userLsid = stripeId;
    mapUpdateInfoList.volumeId = testVolumeId;
    mapUpdateInfoList.vsid = stripeId;
    mapUpdateInfoList.wbLsid = stripeId;
    // given valid block map info for map update
    for (uint32_t index = 0; index < validMapCnt; index++)
    {
        GcBlockMapUpdate validMap = {.rba = index, .vsa = {.stripeId = stripeId, .offset = index}};
        mapUpdateInfoList.blockMapUpdateList.push_back(validMap);
        VirtualBlks vsaRange = {validMap.vsa, 1};
        EXPECT_CALL(*vsaMap, SetVSAsInternal(testVolumeId, index, vsaRange)).Times(1);
    }
    // given invalid block cnt and segment id
    invalidSegCnt.emplace(segId, invalidBlockCnt);
    gcMapUpdate = new GcMapUpdate(stripe, arrayName, mapUpdateInfoList, invalidSegCnt,
                stripeMap, gcStripeManager, eventScheduler, inputEvent, array,
                vsaMap, contextManager, blockAllocator);

    // when execute
    // then set lsa, update occupiedStripeCount, validate blocks, enqueue event
    EXPECT_CALL(*stripe, GetVsid).WillRepeatedly(Return(stripeId));
    EXPECT_CALL(*stripe, GetUserLsid).WillRepeatedly(Return(stripeId));
    EXPECT_CALL(*stripeMap, SetLSA(_, _, _)).Times(1);
    EXPECT_CALL(*contextManager, UpdateOccupiedStripeCount(stripeId)).Times(1);
    VirtualBlkAddr writeVsa = {stripeId, 0};
    VirtualBlks writeVsaRange = {writeVsa, validMapCnt};
    EXPECT_CALL(*blockAllocator, ValidateBlks(writeVsaRange)).Times(1);
    EXPECT_CALL(*eventScheduler, EnqueueEvent(inputEvent)).Times(1);

    EXPECT_TRUE(gcMapUpdate->Execute() == true);
}

} // namespace pos
