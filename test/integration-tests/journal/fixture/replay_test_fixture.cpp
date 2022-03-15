#include "test/integration-tests/journal/fixture/replay_test_fixture.h"

using ::testing::_;
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
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        ReplaySegmentAllocation(firstStripe))
        .Times(AnyNumber());
}

void
ReplayTestFixture::ExpectReplayStripeAllocation(StripeId vsid, StripeId wbLsid)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()),
        SetLSA(vsid, wbLsid, IN_WRITE_BUFFER_AREA))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        ReplayStripeAllocation(wbLsid, vsid))
        .Times(1);
}

void
ReplayTestFixture::ExpectReplayBlockLogsForStripe(int volId, BlockMapList blksToWrite)
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
            EXPECT_CALL(*(mapper->GetVSAMapMock()), SetVSAsWithSyncOpen(volId, rba + offset, blk));
            EXPECT_CALL(*(allocator->GetISegmentCtxMock()), ValidateBlks(blk));
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
ReplayTestFixture::ExpectReplayStripeFlush(StripeTestFixture stripe)
{
    EXPECT_CALL(*(mapper->GetStripeMapMock()), SetLSA(stripe.GetVsid(), stripe.GetUserAddr().stripeId, stripe.GetUserAddr().stripeLoc)).Times(1);

    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        ReplayStripeRelease(stripe.GetWbAddr().stripeId))
        .Times(1);

    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        ReplayStripeFlushed(stripe.GetUserAddr().stripeId))
        .Times(1);
}

void
ReplayTestFixture::ExpectReplayFullStripe(StripeTestFixture stripe)
{
    BlockMapList blksToWrite = stripe.GetBlockMapList();
    // FIXME (huijeong.kim) Due to the limitation we have (see _AddLogInternal)
    // Replaying block map updated log could be non-sequential
    {
        InSequence s;

        ExpectReplaySegmentAllocation(stripe.GetUserAddr().stripeId);
        ExpectReplayStripeAllocation(stripe.GetVsid(), stripe.GetWbAddr().stripeId);
        ExpectReplayStripeFlush(stripe);
    }

    ExpectReplayBlockLogsForStripe(stripe.GetVolumeId(), blksToWrite);
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
            EXPECT_CALL(*(allocator->GetISegmentCtxMock()), InvalidateBlks(blks));
        }
    }
}

void
ReplayTestFixture::ExpectReplayUnflushedActiveStripe(VirtualBlkAddr tail, StripeTestFixture stripe)
{
    EXPECT_CALL(*(allocator->GetWBStripeAllocatorMock()),
        ReconstructActiveStripe(testInfo->defaultTestVol, stripe.GetWbAddr().stripeId, tail, stripe.GetRevMap()))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        SetActiveStripeTail(testInfo->defaultTestVol, tail, stripe.GetWbAddr().stripeId))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);
}

void
ReplayTestFixture::ExpectReplayFlushedActiveStripe(void)
{
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()),
        ResetActiveStripeTail(testInfo->defaultTestVol))
        .Times(1);
    EXPECT_CALL(*(allocator->GetIContextReplayerMock()), ReplaySsdLsid).Times(1);
}

VirtualBlkAddr
ReplayTestFixture::GetNextBlock(VirtualBlks blks)
{
    VirtualBlkAddr nextVsa = blks.startVsa;
    nextVsa.offset += blks.numBlks;
    return nextVsa;
}
} // namespace pos
