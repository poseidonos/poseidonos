#pragma once

#include "src/journal_manager/log/log_event.h"
#include "src/journal_manager/replay/replay_event.h"

namespace pos
{
class StripeReplayStatus;
class IVSAMap;
class IStripeMap;
class IWBStripeCtx;
class ISegmentCtx;
class IBlockAllocator;
class IArrayInfo;

class ReplayEventFactory
{
public:
    ReplayEventFactory(StripeReplayStatus* status, IVSAMap* vsaMap, IStripeMap* stripeMap,
        IContextReplayer* contextReplayer,
        IBlockAllocator* blockAllocator, IArrayInfo* arrayInfo);

    virtual ~ReplayEventFactory(void) = default;

    ReplayEvent* CreateBlockWriteReplayEvent(int volId, BlkAddr startRba, VirtualBlkAddr startVsa, uint64_t numBlks, bool replaySegmentInfo);
    ReplayEvent* CreateStripeMapUpdateReplayEvent(StripeId vsid, StripeAddr dest);
    ReplayEvent* CreateStripeFlushReplayEvent(StripeId vsid, StripeId wbLsid, StripeId userLsid);
    ReplayEvent* CreateStripeAllocationReplayEvent(StripeId vsid, StripeId wbLsid);
    ReplayEvent* CreateSegmentAllocationReplayEvent(StripeId userLsid);

private:
    StripeReplayStatus* status;

    IVSAMap* vsaMap;
    IStripeMap* stripeMap;
    IContextReplayer* contextReplayer;
    IBlockAllocator* blockAllocator;
    IArrayInfo* arrayInfo;
};
} // namespace pos
