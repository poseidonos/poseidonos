#include "src/gc/victim_stripe.h"

#include <gtest/gtest.h>

#include <src/include/address_type.h>
#include <src/include/partition_type.h>
#include <src/include/smart_ptr_type.h>
#include <src/include/meta_const.h>
#include <src/volume/volume_base.h>
#include <src/mapper/include/mapper_const.h>
#include <test/unit-tests/array_models/interface/i_array_info_mock.h>
#include <test/unit-tests/mapper/i_reversemap_mock.h>
#include <test/unit-tests/mapper/i_vsamap_mock.h>
#include <test/unit-tests/mapper/i_stripemap_mock.h>
#include <test/unit-tests/volume/i_volume_io_manager_mock.h>
#include <test/unit-tests/gc/reverse_map_load_completion_mock.h>
#include <test/unit-tests/utils/mock_builder.h>

using ::testing::NiceMock;
using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::Test;

namespace pos
{
class VictimStripeTestFixture : public ::testing::Test
{
public:
    VictimStripeTestFixture(void)
    :victimStripe(nullptr)
    {
    }

    virtual ~VictimStripeTestFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        array = new NiceMock<MockIArrayInfo>;
        reverseMap = new NiceMock<MockIReverseMap>;
        vsaMap = new NiceMock<MockIVSAMap>;
        stripeMap = new NiceMock<MockIStripeMap>;
        volumeManager = new NiceMock<MockIVolumeIoManager>;
        reverseMapLoadCompletionPtr = std::make_shared<NiceMock<MockReverseMapLoadCompletion>>();

        EXPECT_CALL(*array, GetSizeInfo(PartitionType::USER_DATA)).WillOnce(Return((const PartitionLogicalSize*)&partitionLogicalSize));

        ON_CALL(*vsaMap, GetVSAInternal(_, _, _)).WillByDefault([this](int volumeId, BlkAddr startRba, int& caller)
        {
            if (this->call_count++ == 0)
            {
                caller = NEED_RETRY;
            }
            else
            {
                caller = OK_READY;
            }
            VirtualBlkAddr tmpVsa{TEST_SEGMENT_1_BASE_STRIPE_ID, startRba};
            return tmpVsa;
        });

        victimStripe = new VictimStripe(array, reverseMap, vsaMap, stripeMap, volumeManager);
    }

    virtual void
    TearDown(void)
    {
        delete victimStripe;
        delete array;
        delete vsaMap;
        delete stripeMap;
        delete volumeManager;
        delete reverseMap;
    }

protected:
    VictimStripe* victimStripe;
    NiceMock<MockIArrayInfo>* array;
    NiceMock<MockIReverseMap>* reverseMap;
    NiceMock<MockIVSAMap>* vsaMap;
    NiceMock<MockIStripeMap>* stripeMap;
    NiceMock<MockIVolumeIoManager>* volumeManager;
    CallbackSmartPtr reverseMapLoadCompletionPtr;

    PartitionLogicalSize partitionLogicalSize = {
    .minWriteBlkCnt = 0, /* no interesting */
    .blksPerChunk = BLOCKS_IN_CHUNK,
    .blksPerStripe = BLOCKS_IN_CHUNK * 4, // blksPerChunk * chunksPerStripe
    .chunksPerStripe = 4,
    .stripesPerSegment = STRIPES_PER_SEGMENT,
    .totalStripes = 32,
    .totalSegments = 32768
    };
    uint32_t TEST_SEGMENT_1 = 100;
    uint32_t TEST_SEGMENT_1_BASE_STRIPE_ID = TEST_SEGMENT_1 * partitionLogicalSize.stripesPerSegment;
    int call_count{0};
};

TEST_F(VictimStripeTestFixture, Load_)
{
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));
}

TEST_F(VictimStripeTestFixture, GetBlkInfoList_)
{
}

TEST_F(VictimStripeTestFixture, GetBlkInfoListSize_)
{
}

TEST_F(VictimStripeTestFixture, LoadValidBlock_GetVsaRetry)
{
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;

    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillRepeatedly(Return(std::tie(blockOffset, volId)));
    }
    // set victim stripe id
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // given
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID};
    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillRepeatedly(Return(lsa));
    EXPECT_CALL(*volumeManager, IncreasePendingIOCountIfNotZero(volId, VolumeIoType::InternalIo, 1)).WillRepeatedly(Return(0));

    // when loadValidBlock mapper return need retry then return false
    EXPECT_TRUE(victimStripe->LoadValidBlock() == false);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithValidAllBlocks)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;

    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillRepeatedly(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = blockOffset};
        ON_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillByDefault(Return(virtualBlkAddr));
    }

    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID};
    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillRepeatedly(Return(lsa));
    EXPECT_CALL(*volumeManager, IncreasePendingIOCountIfNotZero(volId, VolumeIoType::InternalIo, 1)).WillRepeatedly(Return(0));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == partitionLogicalSize.chunksPerStripe);
    list<BlkInfo> blkInfoList = victimStripe->GetBlkInfoList(0);
    EXPECT_TRUE(blkInfoList.size() == partitionLogicalSize.blksPerChunk);
    uint32_t blockOffset = 0;
    for (auto blockInfo : blkInfoList)
    {
        EXPECT_TRUE(blockInfo.rba == blockOffset);
        EXPECT_TRUE(blockInfo.volID == volId);
        EXPECT_TRUE(blockInfo.vsa.stripeId == TEST_SEGMENT_1_BASE_STRIPE_ID);
        EXPECT_TRUE(blockInfo.vsa.offset == blockOffset);
        blockOffset++;
    }

    blkInfoList.clear();
    blkInfoList = victimStripe->GetBlkInfoList(1);
    EXPECT_TRUE(blkInfoList.size() == partitionLogicalSize.blksPerChunk);

    for (auto blockInfo : blkInfoList)
    {
        EXPECT_TRUE(blockInfo.rba == blockOffset);
        EXPECT_TRUE(blockInfo.volID == volId);
        EXPECT_TRUE(blockInfo.vsa.stripeId == TEST_SEGMENT_1_BASE_STRIPE_ID);
        EXPECT_TRUE(blockInfo.vsa.offset == blockOffset);
        blockOffset++;
    }
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithInvalidReverseMap1)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    uint32_t maxVolCnt = MAX_VOLUME_COUNT;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(INVALID_RBA, maxVolCnt)));
    }
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithInvalidReverseMap2)
{
    // given 
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(INVALID_RBA, volId)));
    }
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);
    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithUnmapVsa)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = UNMAP_STRIPE, .offset = UNMAP_OFFSET};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithInvalidVsa1)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = UNMAP_STRIPE, .offset = blockOffset};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithInvalidVsa2)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = UNMAP_OFFSET};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithUnmapLsa)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = blockOffset};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }

    StripeAddr unMapLsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = UNMAP_STRIPE};
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID};

    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillOnce(Return(unMapLsa)).WillRepeatedly(Return(lsa));
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    EXPECT_CALL(*volumeManager, IncreasePendingIOCountIfNotZero(volId, VolumeIoType::InternalIo, 1)).WillRepeatedly(Return(0));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    uint32_t blkInfoListCnt = victimStripe->GetBlkInfoListSize();
    uint32_t totalValidBlkCnt = 0;
    EXPECT_TRUE(blkInfoListCnt == partitionLogicalSize.chunksPerStripe);
    for (uint32_t index = 0; index < blkInfoListCnt; index++)
    {
        list<BlkInfo> blkInfoList = victimStripe->GetBlkInfoList(index);
        totalValidBlkCnt += blkInfoList.size();
    }
    EXPECT_TRUE(totalValidBlkCnt == ((partitionLogicalSize.blksPerChunk * blkInfoListCnt) - 1));
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithDiffrentLsa)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = blockOffset};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID + 1};
    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillRepeatedly(Return(lsa));
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithDiffrentBlkOffset)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    for (uint32_t blockOffset = 0; blockOffset < blocksPerStripe; blockOffset++)
    {
        EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
        int shouldRetry = CallerEventAndRetry::OK_READY;
        VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = blockOffset + 1};
        EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));
    }
    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID};
    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillRepeatedly(Return(lsa));
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}

TEST_F(VictimStripeTestFixture, LoadValidBlockWithDeletedVolume)
{
    // given
    uint32_t chunksPerStripe = partitionLogicalSize.chunksPerStripe;
    uint32_t blocksPerStripe = partitionLogicalSize.blksPerStripe;
    uint32_t volId = 1;
    uint32_t blockOffset = 0;
    EXPECT_CALL(*reverseMap, GetReverseMapEntry(nullptr, UNMAP_STRIPE, blockOffset)).WillOnce(Return(std::tie(blockOffset, volId)));
    int shouldRetry = CallerEventAndRetry::OK_READY;
    VirtualBlkAddr virtualBlkAddr = {.stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID, .offset = blockOffset};
    EXPECT_CALL(*vsaMap, GetVSAInternal(volId, blockOffset, shouldRetry)).WillOnce(Return(virtualBlkAddr));

    StripeAddr lsa = {.stripeLoc = StripeLoc::IN_USER_AREA, .stripeId = TEST_SEGMENT_1_BASE_STRIPE_ID};
    EXPECT_CALL(*stripeMap, GetLSA(TEST_SEGMENT_1_BASE_STRIPE_ID)).WillRepeatedly(Return(lsa));
    EXPECT_CALL(*volumeManager, IncreasePendingIOCountIfNotZero(volId, VolumeIoType::InternalIo, 1)).WillRepeatedly(Return(2010));
    victimStripe->Load(TEST_SEGMENT_1_BASE_STRIPE_ID, (reverseMapLoadCompletionPtr));

    // when
    EXPECT_TRUE(victimStripe->LoadValidBlock() == true);

    // then
    EXPECT_TRUE(victimStripe->GetBlkInfoListSize() == 0);
}
} // namespace pos
