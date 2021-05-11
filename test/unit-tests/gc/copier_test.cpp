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
#include <test/unit-tests/mapper/i_reversemap_mock.h>
#include <test/unit-tests/mapper/reversemap/reverse_map_mock.h>
#include <test/unit-tests/spdk_wrapper/free_buffer_pool_mock.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;

namespace pos
{
PartitionLogicalSize partitionLogicalSize = {
    .blksPerChunk = 64,
    .blksPerStripe = 2048,
    .chunksPerStripe = 32,
    .stripesPerSegment = 1024,
    .totalStripes = 32,
    .totalSegments = 32768,
};

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
      reverseMapPack(nullptr),
      gcWriteBufferPool(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr),
      stripeCopySubmissionPtr(nullptr),
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
        gcWriteBufferPool = new NiceMock<MockFreeBufferPool>(0, 0);
        gcStripeManager = new NiceMock<MockGcStripeManager>(array, gcWriteBufferPool);

        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                reverseMapPack = new NiceMock<MockReverseMapPack>;
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, reverseMapPack));
            }
        }

        gcBufferPool = new std::vector<FreeBufferPool*>;
        for (uint32_t index = 0; index < GC_BUFFER_COUNT; index++)
        {
            gcBufferPool->push_back(new NiceMock<MockFreeBufferPool>(0, 0));
        }

        meta = new NiceMock<MockCopierMeta>(array, udSize, inUseBitmap, gcStripeManager, victimStripes, gcBufferPool);
        iBlockAllocator = new NiceMock<MockIBlockAllocator>;
        iContextManager = new NiceMock<MockIContextManager>;
        stripeCopySubmissionPtr = std::make_shared<NiceMock<MockStripeCopySubmission>>(baseStripeId, nullptr, copyIndex);
        reverseMapLoadCompletionPtr = std::make_shared<NiceMock<MockReverseMapLoadCompletion>>();
        copier = new Copier(victimId, targetId, gcStatus, array, udSize, meta, iBlockAllocator, iContextManager, stripeCopySubmissionPtr, reverseMapLoadCompletionPtr);
    }

    virtual void
    TearDown(void)
    {
        delete copier;
        delete array;
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
    NiceMock<MockGcStripeManager>* gcStripeManager;
    NiceMock<MockReverseMapPack>* reverseMapPack;
    NiceMock<MockFreeBufferPool>* gcWriteBufferPool;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    std::vector<FreeBufferPool*>* gcBufferPool;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

    CallbackSmartPtr stripeCopySubmissionPtr;
    CallbackSmartPtr reverseMapLoadCompletionPtr;
};

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
