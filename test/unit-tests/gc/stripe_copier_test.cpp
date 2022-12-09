#include "src/gc/stripe_copier.h"

#include <gtest/gtest.h>
#include <src/include/address_type.h>
#include <src/include/partition_type.h>
#include <src/include/smart_ptr_type.h>
#include <test/unit-tests/array_models/dto/partition_logical_size_mock.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/gc/copier_meta_mock.h>
#include <test/unit-tests/gc/stripe_copy_submission_mock.h>
#include <test/unit-tests/gc/victim_stripe_mock.h>
#include <test/unit-tests/lib/bitmap_mock.h>
#include <test/unit-tests/utils/mock_builder.h>
#include <test/unit-tests/event_scheduler/event_scheduler_mock.h>
#include <test/unit-tests/gc/stripe_copier_mock.h>

#include "src/include/meta_const.h"
#include "test/unit-tests/resource_manager/buffer_pool_mock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Test;
using ::testing::AtLeast;

namespace pos
{
static const uint32_t GC_BUFFER_COUNT = 512;
static const uint32_t GC_CONCURRENT_COUNT = 16;
static const uint32_t GC_VICTIM_SEGMENT_COUNT = 2;

class StripeCopierTestFixture : public ::testing::Test
{
public:
    StripeCopierTestFixture(void)
    : array(nullptr),
      meta(nullptr),
      inUseBitmap(nullptr),
      victimStripes(nullptr),
      gcBufferPool(nullptr)
    {
    }

    virtual ~StripeCopierTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        victimId = 0;
        baseStripeId = 0;
        copyIndex = 0;
        array = new NiceMock<MockIArrayInfo>;
        EXPECT_CALL(*array, GetSizeInfo(_)).WillRepeatedly(Return(&partitionLogicalSize));

        victimStripes = new std::vector<std::vector<VictimStripe*>>;
        victimStripes->resize(GC_VICTIM_SEGMENT_COUNT);
        for (uint32_t stripeIndex = 0; stripeIndex < GC_VICTIM_SEGMENT_COUNT; stripeIndex++)
        {
            for (uint32_t i = 0; i < partitionLogicalSize.stripesPerSegment; i++)
            {
                (*victimStripes)[stripeIndex].push_back(new NiceMock<MockVictimStripe>(array, nullptr, nullptr, nullptr, nullptr));
            }
        }
        BufferInfo info = {
            .owner = "stripe_copier_test",
            .size = 256,
            .count =  GC_BUFFER_COUNT
        };

        gcBufferPool = new NiceMock<MockBufferPool>(info, 0, nullptr);
        meta = new NiceMock<MockCopierMeta>(array, udSize, inUseBitmap, nullptr, victimStripes, gcBufferPool);
    }

    virtual void
    TearDown(void)
    {
        delete stripeCopier;
        delete array;
        delete meta;
    }

protected:
    StripeCopier* stripeCopier;

    SegmentId victimId;
    StripeId baseStripeId;
    uint32_t copyIndex;

    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockCopierMeta>* meta;
    NiceMock<MockBitMapMutex>* inUseBitmap;

    std::vector<std::vector<VictimStripe*>>* victimStripes;
    BufferPool* gcBufferPool = nullptr;

    EventSmartPtr mockCopyEvent;
    EventSmartPtr mockStripeCopier;
    NiceMock<MockEventScheduler>* eventScheduler;

    const PartitionLogicalSize* udSize = &partitionLogicalSize;

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

TEST_F(StripeCopierTestFixture, Execute_testIfExecuteFailsWhenFailedToLoadValidBlocks)
{
    stripeCopier = new StripeCopier(baseStripeId, meta, copyIndex, mockCopyEvent,
                        mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetVictimStripe(copyIndex, baseStripeId % STRIPES_PER_SEGMENT)).WillOnce(Return(&victimStripe));
    EXPECT_CALL(victimStripe, LoadValidBlock).WillOnce(Return(false));
    EXPECT_TRUE(stripeCopier->Execute() == false);
}

TEST_F(StripeCopierTestFixture, Execute_testIfExecuteSucceedsEvenWhenBlockInfoListIsEmpty)
{
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    stripeCopier = new StripeCopier(baseStripeId, meta, copyIndex, mockCopyEvent,
                        mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetVictimStripe(copyIndex, baseStripeId % STRIPES_PER_SEGMENT)).WillRepeatedly(Return(&victimStripe));
    EXPECT_CALL(victimStripe, LoadValidBlock).WillOnce(Return(true));
    EXPECT_CALL(victimStripe, GetBlkInfoListSize).WillOnce(Return(0));
    EXPECT_CALL(*meta, SetStartCopyStripes).Times(1);
    EXPECT_CALL(*meta, GetStripePerSegment).WillOnce(Return(STRIPES_PER_SEGMENT));
    EXPECT_CALL(*eventScheduler, EnqueueEvent(mockStripeCopier)).Times(1);

    EXPECT_TRUE(stripeCopier->Execute() == true);

    delete eventScheduler;
    mockStripeCopier = nullptr;
}

TEST_F(StripeCopierTestFixture, Execute_testIfExecuteFailsWhenBufferAllocationFails)
{
    uint32_t testRba = 200;
    uint32_t testVolId = 1;
    uint32_t testStripeId = 100;
    uint64_t testStripeOffset = 30;
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    mockCopyEvent = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    stripeCopier = new StripeCopier(baseStripeId, meta, copyIndex, mockCopyEvent,
                        mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetVictimStripe(copyIndex, baseStripeId % STRIPES_PER_SEGMENT)).WillRepeatedly(Return(&victimStripe));
    EXPECT_CALL(victimStripe, LoadValidBlock).WillOnce(Return(true));
    EXPECT_CALL(victimStripe, GetBlkInfoListSize).WillOnce(Return(1));
    std::vector<void*> emptyVector;
    EXPECT_CALL(*meta, GetBuffers(_,_)).WillOnce(testing::SetArgPointee<1>(emptyVector));
    EXPECT_TRUE(stripeCopier->Execute() == false);

    delete eventScheduler;
    mockStripeCopier = nullptr;
}

TEST_F(StripeCopierTestFixture, Execute_testIfExecuteSucceedsWhenThereIsOneValidBlock)
{
    uint32_t testRba = 200;
    uint32_t testVolId = 1;
    uint32_t testStripeId = 100;
    uint64_t testStripeOffset = 30;
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    mockCopyEvent = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    stripeCopier = new StripeCopier(baseStripeId, meta, copyIndex, mockCopyEvent,
                        mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetVictimStripe(copyIndex, baseStripeId % STRIPES_PER_SEGMENT)).WillRepeatedly(Return(&victimStripe));
    EXPECT_CALL(victimStripe, LoadValidBlock).WillOnce(Return(true));
    EXPECT_CALL(victimStripe, GetBlkInfoListSize).WillOnce(Return(1));
    std::vector<void*> oneBuffer{(void*)0x20000000};
    EXPECT_CALL(*meta, GetBuffers(_,_)).WillOnce(testing::SetArgPointee<1>(oneBuffer));
    std::list<BlkInfo> blkInfoList;
    BlkInfo blkInfo = {.rba = testRba, .volID = testVolId,
                .vsa = {.stripeId = testStripeId, .offset = testStripeOffset}};
    blkInfoList.push_back(blkInfo);
    EXPECT_CALL(victimStripe, GetBlkInfoList(0)).WillOnce(ReturnRef(blkInfoList));

    EXPECT_CALL(*meta, SetStartCopyStripes).Times(1);
    EXPECT_CALL(*meta, GetStripePerSegment).WillOnce(Return(STRIPES_PER_SEGMENT));
    EXPECT_CALL(*eventScheduler, EnqueueEvent(_)).Times(2);

    EXPECT_TRUE(stripeCopier->Execute() == true);

    delete eventScheduler;
    mockStripeCopier = nullptr;
    mockCopyEvent = nullptr;
}

TEST_F(StripeCopierTestFixture, Execute_CopyOneValidBlockCreateCopyEvent)
{
    uint32_t testRba = 200;
    uint32_t testVolId = 1;
    uint32_t testStripeId = 100;
    uint64_t testStripeOffset = 30;
    eventScheduler = new NiceMock<MockEventScheduler>;
    mockStripeCopier = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    mockCopyEvent = std::make_shared<NiceMock<MockStripeCopier>>(baseStripeId, meta, copyIndex,
                        mockCopyEvent, nullptr, eventScheduler);
    stripeCopier = new StripeCopier(baseStripeId, meta, copyIndex, mockCopyEvent,
                        mockStripeCopier, eventScheduler);

    NiceMock<MockVictimStripe> victimStripe(array, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*meta, GetVictimStripe(copyIndex, baseStripeId % STRIPES_PER_SEGMENT)).WillRepeatedly(Return(&victimStripe));
    EXPECT_CALL(victimStripe, LoadValidBlock).WillOnce(Return(true));
    EXPECT_CALL(victimStripe, GetBlkInfoListSize).WillOnce(Return(1));
    std::vector<void*> oneBuffer{(void*)0x20000000};
    EXPECT_CALL(*meta, GetBuffers(_,_)).WillOnce(testing::SetArgPointee<1>(oneBuffer));
    std::list<BlkInfo> blkInfoList;
    BlkInfo blkInfo = {.rba = testRba, .volID = testVolId,
                .vsa = {.stripeId = testStripeId, .offset = testStripeOffset}};
    blkInfoList.push_back(blkInfo);
    EXPECT_CALL(victimStripe, GetBlkInfoList(0)).WillOnce(ReturnRef(blkInfoList));
    EXPECT_CALL(*eventScheduler, EnqueueEvent(mockCopyEvent)).Times(1);

    EXPECT_CALL(*meta, SetStartCopyStripes).Times(1);
    EXPECT_CALL(*meta, GetStripePerSegment).WillOnce(Return(STRIPES_PER_SEGMENT));
    EXPECT_CALL(*eventScheduler, EnqueueEvent(mockStripeCopier)).Times(1);

    EXPECT_TRUE(stripeCopier->Execute() == true);

    delete eventScheduler;
    mockStripeCopier = nullptr;
    mockCopyEvent = nullptr;
}
} // namespace pos
