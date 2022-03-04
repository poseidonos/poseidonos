#include "src/gc/copier.h"

#include <gtest/gtest.h>
#include <src/include/address_type.h>
#include <src/include/partition_type.h>
#include <src/include/smart_ptr_type.h>
#include <test/unit-tests/allocator/i_block_allocator_mock.h>
#include <test/unit-tests/allocator/i_context_manager_mock.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/copier_meta_mock.h>
#include <test/unit-tests/gc/gc_status_mock.h>
#include <test/unit-tests/gc/gc_stripe_manager_mock.h>
#include <test/unit-tests/gc/reverse_map_load_completion_mock.h>
#include <test/unit-tests/gc/stripe_copy_submission_mock.h>
#include <test/unit-tests/gc/victim_stripe_mock.h>
#include <test/unit-tests/lib/bitmap_mock.h>
#include <test/unit-tests/sys_event/volume_event_publisher_mock.h>
#include <test/unit-tests/mapper/i_reversemap_mock.h>
#include <test/unit-tests/mapper/reversemap/reverse_map_mock.h>
#include <test/unit-tests/utils/mock_builder.h>
#include <test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h>
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"
#include "test/unit-tests/resource_manager/memory_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/segment_ctx/segment_ctx_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;
using ::testing::AtLeast;

namespace pos
{
static const uint32_t GC_BUFFER_COUNT = 512;
static const uint32_t GC_CONCURRENT_COUNT = 16;
static const uint32_t GC_VICTIM_SEGMENT_COUNT = 2;

class CopierTestFixture : public ::testing::Test
{
public:
    CopierTestFixture(void)
    : gcStatus(nullptr),
      array(nullptr),
      meta(nullptr),
      iBlockAllocator(nullptr),
      iContextManager(nullptr),
      inUseBitmap(nullptr),
      gcStripeManager(nullptr),
      iReverseMap(nullptr),
      volumeEventPublisher(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr),
      stripeCopySubmissionPtr(nullptr),
      gcCtx(nullptr),
      segCtx(nullptr),
      reverseMapLoadCompletionPtr(nullptr)
    {
    }

    virtual ~CopierTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        victimId = 0;
        targetId = 0;
        baseStripeId = 0;
        copyIndex = 0;
        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        gcStatus = new NiceMock<MockGcStatus>;
        volumeEventPublisher = new NiceMock<MockVolumeEventPublisher>();
        memoryManager = new MockMemoryManager();
        EXPECT_CALL(*memoryManager, CreateBufferPool).WillRepeatedly(Return(nullptr));
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, volumeEventPublisher, memoryManager);
        iReverseMap = new NiceMock<MockIReverseMap>();
        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, iReverseMap, nullptr, nullptr, nullptr));
            }
        }

        gcBufferPool = new std::vector<BufferPool*>;
        for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
        {
            BufferInfo info;
            gcBufferPool->push_back(new NiceMock<MockBufferPool>(info, 0, nullptr));
        }

        meta = new NiceMock<MockCopierMeta>(array, udSize, inUseBitmap, gcStripeManager, victimStripes, gcBufferPool);
        iBlockAllocator = new NiceMock<MockIBlockAllocator>;
        iContextManager = new NiceMock<MockIContextManager>;
        stripeCopySubmissionPtr = std::make_shared<NiceMock<MockStripeCopySubmission>>(baseStripeId, nullptr, copyIndex);
        reverseMapLoadCompletionPtr = std::make_shared<NiceMock<MockReverseMapLoadCompletion>>();
        copier = new Copier(victimId, targetId, gcStatus, array, udSize, meta, iBlockAllocator, iContextManager, stripeCopySubmissionPtr, reverseMapLoadCompletionPtr);

        gcCtx = new NiceMock<MockGcCtx>;
        segCtx = new NiceMock<MockSegmentCtx>;

        EXPECT_CALL(*iContextManager, GetSegmentCtx).WillRepeatedly(Return(segCtx));
        EXPECT_CALL(*iContextManager, GetGcCtx).WillRepeatedly(Return(gcCtx));
    }

    virtual void
    TearDown(void)
    {
        delete copier;
        delete array;
        delete gcCtx;
        delete segCtx;
        delete iBlockAllocator;
        delete iContextManager;
        delete memoryManager;
    }

protected:
    Copier* copier;

    SegmentId victimId;
    SegmentId targetId;
    StripeId baseStripeId;
    uint32_t copyIndex;

    NiceMock<MockGcStatus>* gcStatus;
    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockCopierMeta>* meta;
    NiceMock<MockIBlockAllocator>* iBlockAllocator;
    NiceMock<MockIContextManager>* iContextManager;
    NiceMock<MockBitMapMutex>* inUseBitmap;
    NiceMock<MockVolumeEventPublisher>* volumeEventPublisher;
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockIReverseMap>* iReverseMap;
    NiceMock<MockGcCtx>* gcCtx;
    NiceMock<MockSegmentCtx>* segCtx;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    std::vector<BufferPool*>* gcBufferPool;
    MockMemoryManager* memoryManager;
    CpuSetArray cpuSetArray;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    CallbackSmartPtr stripeCopySubmissionPtr;
    CallbackSmartPtr reverseMapLoadCompletionPtr;
    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0/* not interesting */,
    .blksPerChunk = 4,
    .blksPerStripe = 16,
    .chunksPerStripe = 4,
    .stripesPerSegment = 2,
    .totalStripes = 300,
    .totalSegments = 300,
    };

    uint32_t TEST_SEGMENT_1 = 100;
    uint32_t TEST_SEGMENT_2 = 200;
    uint32_t TEST_SEGMENT_1_BASE_STRIPE_ID = TEST_SEGMENT_1 * partitionLogicalSize.stripesPerSegment;
    uint32_t TEST_SEGMENT_2_BASE_STRIPE_ID = TEST_SEGMENT_2 * partitionLogicalSize.stripesPerSegment;
};

TEST_F(CopierTestFixture, Execute_NoGcMode)
{
    // check gc mode when no gc situation
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NO_GC));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testNormalGcAndGetUnmapSegmentId)
{
    // check gc mode when normal gc situation and do not find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testNormalGcAndGetTestSegmentId)
{
    // check gc mode when normal gc situation and find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testUrgentGcAndGetTestSegmentId)
{
    // check gc mode when normal gc situation and find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_URGENT_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testPrepareState)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode when normal gc situation and find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // prepare gc state
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testCompleteState)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode when normal gc situation and find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // prepare gc state
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state done
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(true));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testDisableThresholdCheck)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode state and disable threshold check test
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NO_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_TRUE(copier->IsEnableThresholdCheck());
    copier->DisableThresholdCheck();
    EXPECT_FALSE(copier->IsEnableThresholdCheck());
    EXPECT_FALSE(copier->Execute());

    // prepare gc test
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(true));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testStopWhenPrepareState)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    // check gc mode state and disable threshold check test
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // gc stop done test
    EXPECT_CALL(*meta, IsAllVictimSegmentCopyDone()).WillRepeatedly(Return(true));
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillOnce(Return(false));
    copier->Stop();
    EXPECT_TRUE(copier->IsStopped());
    EXPECT_FALSE(copier->Execute());

    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillRepeatedly(Return(true));
    EXPECT_FALSE(copier->Execute());
    EXPECT_FALSE(copier->IsStopped());
    EXPECT_FALSE(copier->Execute());
    copier->ReadyToEnd();
    EXPECT_TRUE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testStopWhenCompleteState)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode state and disable threshold check test
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // prepare gc test
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    // wait gc stop test
    EXPECT_CALL(*meta, IsAllVictimSegmentCopyDone()).WillOnce(Return(false));
    copier->Stop();
    EXPECT_TRUE(copier->IsStopped());
    EXPECT_FALSE(copier->Execute());

    // gc stop done test
    EXPECT_CALL(*meta, IsAllVictimSegmentCopyDone()).WillRepeatedly(Return(true));
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillRepeatedly(Return(true));
    EXPECT_FALSE(copier->Execute());
    EXPECT_FALSE(copier->IsStopped());
    EXPECT_FALSE(copier->Execute());
    copier->ReadyToEnd();
    EXPECT_TRUE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testPauseAndResume)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode when normal gc situation and find victim segment
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // gc puase test
    EXPECT_FALSE(copier->IsPaused());
    copier->Pause();
    EXPECT_TRUE(copier->IsPaused());
    EXPECT_FALSE(copier->Execute());

    // gc resume test
    copier->Resume();
    EXPECT_FALSE(copier->IsPaused());

    // prepare gc state
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state done
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(true));
    EXPECT_FALSE(copier->Execute());
}

TEST_F(CopierTestFixture, Execute_testComplexityScenario)
{
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
    {
        EXPECT_CALL(*meta, GetVictimStripe(0, i)).WillRepeatedly(Return((*victimStripes)[0][i]));
        EXPECT_CALL(*meta, GetVictimStripe(1, i)).WillRepeatedly(Return((*victimStripes)[1][i]));
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[0][i]), Load(TEST_SEGMENT_1_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
        EXPECT_CALL(*dynamic_cast<MockVictimStripe*>((*victimStripes)[1][i]), Load(TEST_SEGMENT_2_BASE_STRIPE_ID + i, reverseMapLoadCompletionPtr)).Times(1);
    }

    // check gc mode when no gc situation
    int numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NO_GC));
    EXPECT_FALSE(copier->Execute());

    // check gc mode when normal gc situation and do not find victim segment
    numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(UNMAP_SEGMENT));
    EXPECT_FALSE(copier->Execute());

    // check gc mode when normal gc situation and find victim segment
    numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_NORMAL_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_1));
    EXPECT_FALSE(copier->Execute());

    // prepare gc state
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(0));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(false));
    EXPECT_CALL(*meta, IsCopyDone()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state done
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(true));
    EXPECT_FALSE(copier->Execute());

    // check gc mode when normal gc situation and find victim segment
    numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillOnce(Return(GcMode::MODE_URGENT_GC));
    EXPECT_CALL(*iContextManager, AllocateGCVictimSegment()).WillOnce(Return(TEST_SEGMENT_2));
    EXPECT_FALSE(copier->Execute());

    // prepare gc state
    EXPECT_CALL(*meta, SetInUseBitmap()).WillOnce(Return(1));
    EXPECT_FALSE(copier->Execute());

    // wait gc completion state done
    EXPECT_CALL(*meta, IsSynchronized()).WillOnce(Return(true));
    EXPECT_FALSE(copier->Execute());

    // gc puase test
    EXPECT_FALSE(copier->IsPaused());
    copier->Pause();
    EXPECT_TRUE(copier->IsPaused());
    EXPECT_FALSE(copier->Execute());

    // gc resume test
    copier->Resume();
    EXPECT_FALSE(copier->IsPaused());
    numFreeSegments = segCtx->GetNumOfFreeSegment();
    EXPECT_CALL(*gcCtx, GetCurrentGcMode(numFreeSegments)).WillRepeatedly(Return(GcMode::MODE_NO_GC));
    EXPECT_FALSE(copier->Execute());

    // gc stop test
    copier->Stop();
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillOnce(Return(false));
    EXPECT_FALSE(copier->Execute());

    // gc ready to end
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillRepeatedly(Return(true));
    EXPECT_FALSE(copier->Execute());
    EXPECT_FALSE(copier->IsStopped());
    EXPECT_FALSE(copier->Execute());
    copier->ReadyToEnd();
    EXPECT_TRUE(copier->Execute());
}

TEST_F(CopierTestFixture, Stop_testIfStopped)
{
    copier->Stop();
    EXPECT_TRUE(copier->IsStopped());
}

TEST_F(CopierTestFixture, Execute_testWhenCopierStoppedGcStripeManagerNotFinishued)
{
    // Given
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillRepeatedly(Return(false));

    copier->Stop();
    EXPECT_FALSE(copier->Execute());
    EXPECT_TRUE(copier->GetCopybackState() != COPIER_READY_TO_END_STATE);
}

TEST_F(CopierTestFixture, Execute_testWhenCopierStoppedGcStripeManagerFinished)
{
    // Given
    EXPECT_CALL(*meta, GetGcStripeManager()).WillRepeatedly(Return(gcStripeManager));
    EXPECT_CALL(*gcStripeManager, IsAllFinished()).WillRepeatedly(Return(true));

    copier->Stop();
    EXPECT_FALSE(copier->Execute());
    EXPECT_TRUE(copier->GetCopybackState() == COPIER_READY_TO_END_STATE);
}

TEST_F(CopierTestFixture, Execute_testWhenCopierPaused)
{
    copier->Pause();
    EXPECT_FALSE(copier->Execute());
}
} // namespace pos
