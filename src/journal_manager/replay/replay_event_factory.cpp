#include "src/journal_manager/replay/replay_event_factory.h"

namespace pos
{
ReplayEventFactory::ReplayEventFactory(StripeReplayStatus* status,
    IVSAMap* vsaMap, IStripeMap* stripeMap, IWBStripeCtx* wbStripeCtx,
    ISegmentCtx* segmentCtx, IBlockAllocator* blockAllocator, IArrayInfo* info)
: status(status),
  vsaMap(vsaMap),
  stripeMap(stripeMap),
  wbStripeCtx(wbStripeCtx),
  segmentCtx(segmentCtx),
  blockAllocator(blockAllocator),
  arrayInfo(info)
{
}

ReplayEvent*
ReplayEventFactory::CreateBlockWriteReplayEvent(int volId, BlkAddr startRba,
    VirtualBlkAddr startVsa, uint64_t numBlks)
{
    ReplayBlockMapUpdate* blockMapUpdate
        = new ReplayBlockMapUpdate(vsaMap, blockAllocator, status,
        volId, startRba, startVsa, numBlks);
    return blockMapUpdate;
}

ReplayEvent*
ReplayEventFactory::CreateStripeMapUpdateReplayEvent(StripeId vsid, StripeAddr dest)
{
    ReplayEvent* stripeMapUpdate
        = new ReplayStripeMapUpdate(stripeMap, status, vsid, dest);
    return stripeMapUpdate;
}

ReplayEvent*
ReplayEventFactory::CreateStripeFlushReplayEvent(StripeId vsid, StripeId wbLsid, StripeId userLsid)
{
    ReplayEvent* stripeFlush = new ReplayStripeFlush(wbStripeCtx, segmentCtx,
        status, vsid, wbLsid, userLsid);
    return stripeFlush;
}

ReplayEvent*
ReplayEventFactory::CreateStripeAllocationReplayEvent(StripeId vsid, StripeId wbLsid)
{
    ReplayEvent* stripeAllocation = new ReplayStripeAllocation(stripeMap,
        wbStripeCtx, status, vsid, wbLsid);
    return stripeAllocation;
}

ReplayEvent*
ReplayEventFactory::CreateSegmentAllocationReplayEvent(StripeId userLsid)
{
    ReplayEvent* segmentAllocation = new ReplaySegmentAllocation(segmentCtx,
        arrayInfo, status, userLsid);
    return segmentAllocation;
}

} // namespace pos
