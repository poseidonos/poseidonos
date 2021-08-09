#include "src/gc/gc_map_update_request.h"

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

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
namespace pos
{

class GcMapUpdateRequestTestFixture : public ::testing::Test
{
public:
    GcMapUpdateRequestTestFixture(void)
    : gcMapUpdateRequest(nullptr),
      array(nullptr),
      gcStripeManager(nullptr),
      affinityManager(nullptr),
      gcWriteBufferPool(nullptr),
      volumeEventPublisher(nullptr)
    {
    }

    virtual ~GcMapUpdateRequestTestFixture(void)
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
    }

    virtual void
    TearDown(void)
    {
        delete gcMapUpdateRequest;
        delete array;
        delete affinityManager;
        delete volumeEventPublisher;
        delete gcStripeManager;
        delete stripe;
        delete rbaStateManager;
        delete vsaMap;
        delete journal;
        delete stripeMap;
        delete eventScheduler;

        inputEvent = nullptr;
    }

protected:
    GcMapUpdateRequest* gcMapUpdateRequest;

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

TEST_F(GcMapUpdateRequestTestFixture, Execute_testGcMapUpdatRequestWithAllValidBlkInfoWhenJournalIsEnabled)
{
    gcMapUpdateRequest = new GcMapUpdateRequest(stripe, arrayName, gcStripeManager, inputEvent, stripeMap,
                    vsaMap, journal, eventScheduler, array);

    uint32_t stripeId = 100;
    EXPECT_CALL(*stripe, GetVsid).WillOnce(Return(stripeId));
    // given copied blk reversemap, vsa map and victim vsa map
    for (uint32_t index = 0; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillRepeatedly(Return(revMapEntry));
        VirtualBlkAddr vsa = {.stripeId = stripeId, .offset = index};
        EXPECT_CALL(*vsaMap, GetVSAInternal(testVolumeId, index, _)).WillOnce(Return(vsa));
        EXPECT_CALL(*stripe, GetVictimVsa(index)).WillOnce(Return(vsa));
        MpageList dirty;
        dirty.insert(index);

        EXPECT_CALL(*vsaMap, GetDirtyVsaMapPages(testVolumeId, index, 1)).WillOnce(Return(dirty));
    }

    // when journal is enabled
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*stripe, GetUserLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*journal, IsEnabled(arrayName)).WillOnce(Return(true));
    EXPECT_CALL(*stripeMap, GetDirtyStripeMapPages(stripeId)).Times(1);

    // then call add Gc stripe flushed log using journal write
    NiceMock<MockIJournalWriter> journalWriter;
    EXPECT_CALL(*journal, GetWriter(arrayName)).WillOnce(Return(&journalWriter));
    EXPECT_CALL(journalWriter, AddGcStripeFlushedLog(_, _, _)).Times(1);
    EXPECT_TRUE(gcMapUpdateRequest->Execute() == true);
}

TEST_F(GcMapUpdateRequestTestFixture, Execute_testGcMapUpdatRequestWithAllValidBlkInfoWhenJournalIsDisabled)
{
    gcMapUpdateRequest = new GcMapUpdateRequest(stripe, arrayName, gcStripeManager, inputEvent, stripeMap,
                    vsaMap, journal, eventScheduler, array);

    uint32_t stripeId = 100;
    EXPECT_CALL(*stripe, GetVsid).WillOnce(Return(stripeId));
    // given copied blk reversemap, vsa map and victim vsa map
    for (uint32_t index = 0; index < partitionLogicalSize.blksPerStripe; index++)
    {
        std::pair<uint32_t, uint32_t> revMapEntry = {index, testVolumeId};
        EXPECT_CALL(*stripe, GetReverseMapEntry(index)).WillRepeatedly(Return(revMapEntry));
        VirtualBlkAddr vsa = {.stripeId = stripeId, .offset = index};
        EXPECT_CALL(*vsaMap, GetVSAInternal(testVolumeId, index, _)).WillOnce(Return(vsa));
        EXPECT_CALL(*stripe, GetVictimVsa(index)).WillOnce(Return(vsa));
        MpageList dirty;
        dirty.insert(index);

        EXPECT_CALL(*vsaMap, GetDirtyVsaMapPages(testVolumeId, index, 1)).WillOnce(Return(dirty));
    }

    // when journal is disabled
    EXPECT_CALL(*stripe, GetWbLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*stripe, GetUserLsid).WillOnce(Return(stripeId));
    EXPECT_CALL(*journal, IsEnabled(arrayName)).WillOnce(Return(false));
    EXPECT_CALL(*stripeMap, GetDirtyStripeMapPages(stripeId)).Times(0);

    // then not call journal write, call enqueueEvent using eventscheduler
    NiceMock<MockIJournalWriter> journalWriter;
    EXPECT_CALL(*journal, GetWriter(arrayName)).Times(0);
    EXPECT_CALL(journalWriter, AddGcStripeFlushedLog(_, _, _)).Times(0);
    EXPECT_CALL(*eventScheduler, EnqueueEvent(inputEvent)).Times(1);
    EXPECT_TRUE(gcMapUpdateRequest->Execute() == true);
}
} // namespace pos
