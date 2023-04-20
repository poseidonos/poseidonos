#include "replay_test_fixture.h"

#include "test/integration-tests/journal/fake/i_context_replayer_fake.h"
#include "test/integration-tests/journal/fake/segment_ctx_fake.h"
#include "test/integration-tests/journal/fake/wbstripe_allocator_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::AnyNumber;
using ::testing::InSequence;
using ::testing::Return;

namespace pos
{
ReplayTestFixture::ReplayTestFixture(MockMapper* mapper,
    AllocatorMock* allocator, TestInfo* testInfo)
: mapper(mapper),
  allocator(allocator),
  testInfo(testInfo)
{
}

ReplayTestFixture::~ReplayTestFixture(void)
{
}

void
ReplayTestFixture::ExpectReturningUnmapStripes(void)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        GetLSA)
        .WillRepeatedly(Return(unmapAddr));
}

void
ReplayTestFixture::ExpectReturningStripeAddr(StripeId vsid, StripeAddr addr)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        GetLSA(vsid))
        .WillRepeatedly(Return(addr));
}

void
ReplayTestFixture::ExpectReplaySegmentAllocation(StripeId userLsid)
{
    StripeId firstStripe = userLsid / testInfo->numStripesPerSegment * testInfo->numStripesPerSegment;
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
        ReplaySegmentAllocation(firstStripe))
        .Times(AnyNumber());
}

void
ReplayTestFixture::ExpectReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
        ReplayStripeAllocation(wbLsid, vsid))
        .Times(1);
}

void
ReplayTestFixture::ExpectReplayBlockLogsForStripe(int volId, BlockMapList blksToWrite, bool needToReplaySegment)
{
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    // InSequence s;

    for (auto blk : blksToWrite)
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks blks = std::get<1>(blk);

        for (uint32_t offset = 0; offset < blks.numBlks; offset++)
        {
            VirtualBlks blk = _GetBlock(blks, offset);
            EXPECT_CALL(*(mapper->GetVSAMapFake()), SetVSAsWithSyncOpen(volId, rba + offset, blk));
            if (needToReplaySegment == true)
            {
                EXPECT_CALL(*(allocator->GetIContextReplayerFake()), ReplayBlockValidated(blk));
            }
        }
    }
}

VirtualBlks
ReplayTestFixture::_GetBlock(VirtualBlks blks, uint32_t offset)
{
    VirtualBlks blk = {
        .startVsa = {
            .stripeId = blks.startVsa.stripeId,
            .offset = blks.startVsa.offset + offset},
        .numBlks = 1};
    return blk;
}

void
ReplayTestFixture::ExpectReplayStripeFlush(StripeTestFixture stripe, bool needToReplaySegment)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()), SetLSA(stripe.GetVsid(), stripe.GetUserAddr().stripeId, stripe.GetUserAddr().stripeLoc)).Times(1);

    if (needToReplaySegment == true)
    {
        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplayStripeRelease(stripe.GetWbAddr().stripeId))
            .Times(1);

        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplayStripeFlushed(stripe.GetUserAddr().stripeId))
            .Times(1);
    }
}

void
ReplayTestFixture::ExpectReplayFullStripe(StripeTestFixture stripe, bool needToReplaySegment)
{
    BlockMapList blksToWrite = stripe.GetBlockMapList();
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    {
        InSequence s;

        ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        ExpectReplayStripeFlush(stripe, needToReplaySegment);
    }

    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), blksToWrite, needToReplaySegment);
}

void
ReplayTestFixture::ExpectReplayFullStripe(StripeTestFixture stripe, bool needToReplaySegmentForBlockMap, bool needToReplaySegmentForStripeMap)
{
    BlockMapList blksToWrite = stripe.GetBlockMapList();
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    {
        InSequence s;

        ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        ExpectReplayStripeFlush(stripe, needToReplaySegmentForStripeMap);
    }

    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), blksToWrite, needToReplaySegmentForBlockMap);
}

void
ReplayTestFixture::ExpectReplayReusedStripe(StripeTestFixture stripe)
{
    auto it = usedVisd.find(stripe.GetVsid());
    if (it == usedVisd.end())
    {
        StripeId firstStripe = stripe.GetUserAddr().stripeId / testInfo->numStripesPerSegment * testInfo->numStripesPerSegment;
        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplaySegmentAllocation(firstStripe))
            .Times(AtLeast(0));

        EXPECT_CALL(*(mapper->GetStripeMapMock()),
            SetLSA(stripe.GetVsid(), stripe.GetWbAddr().stripeId, IN_WRITE_BUFFER_AREA))
            .Times(AtLeast(1));
        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplayStripeAllocation(stripe.GetWbAddr().stripeId, stripe.GetVsid()))
            .Times(AtLeast(1));

        EXPECT_CALL(*(mapper->GetStripeMapMock()),
            SetLSA(stripe.GetVsid(), stripe.GetUserAddr().stripeId, stripe.GetUserAddr().stripeLoc))
            .Times(AtLeast(1));
        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplayStripeRelease(stripe.GetWbAddr().stripeId))
            .Times(AtLeast(1));

        EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
            ReplayStripeFlushed(stripe.GetUserAddr().stripeId))
            .Times(AtLeast(1));
        usedVisd.insert(stripe.GetVsid());
    }

    for (auto blk : stripe.GetBlockMapList())
    {
        BlkAddr rba = std::get<0>(blk);
        VirtualBlks blks = std::get<1>(blk);
        for (uint32_t offset = 0; offset < blks.numBlks; offset++)
        {
            VirtualBlks blk = _GetBlock(blks, offset);
            EXPECT_CALL(*(mapper->GetVSAMapFake()), SetVSAsWithSyncOpen(stripe.GetVolumeId(), rba + offset, blk));
            if (it == usedVisd.end())
            {
                EXPECT_CALL(*(allocator->GetIContextReplayerFake()), ReplayBlockValidated(blk)).Times(AtLeast(1));
            }
        }
    }
}

void
ReplayTestFixture::ExpectReplayOverwrittenBlockLog(StripeTestFixture stripe)
{
    BlockMapList writtenVsas = stripe.GetBlockMapList();
    ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
    ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), writtenVsas);

    for (auto vsa = writtenVsas.begin(); vsa != writtenVsas.end() - 1; vsa++)
    {
        for (uint32_t blockOffset = 0; blockOffset < (*vsa).second.numBlks; blockOffset++)
        {
            VirtualBlks blks = _GetBlock((*vsa).second, blockOffset);
            EXPECT_CALL(*(allocator->GetIContextReplayerFake()), ReplayBlockInvalidated(blks, false));
        }
    }
}

void
ReplayTestFixture::ExpectReplayUnflushedActiveStripe(VirtualBlkAddr tail, StripeTestFixture stripe)
{
    EXPECT_CALL(*(allocator->GetWBStripeAllocatorMock()),
        ReconstructActiveStripe(testInfo->defaultTestVol, stripe.GetWbAddr().stripeId, tail, stripe.GetRevMap()))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
        SetActiveStripeTail(testInfo->defaultTestVol, tail, stripe.GetWbAddr().stripeId))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()), ReplaySsdLsid).Times(1);
}

void
ReplayTestFixture::ExpectReplayFlushedActiveStripe(void)
{
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()),
        ResetActiveStripeTail(testInfo->defaultTestVol))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerFake()), ReplaySsdLsid).Times(1);
}

VirtualBlkAddr
ReplayTestFixture::GetNextBlock(VirtualBlks blks)
{
    VirtualBlkAddr nextVsa = blks.startVsa;
    nextVsa.offset += blks.numBlks;
    return nextVsa;
}

void
ReplayTestFixture::UpdateMapper(MockMapper* _mapper)
{
    mapper = _mapper;
}
} // namespace pos
