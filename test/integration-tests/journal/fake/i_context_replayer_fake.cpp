#include "i_context_replayer_fake.h"

#include "test/integration-tests/journal/fake/segment_ctx_fake.h"

namespace pos
{
IContextReplayerFake::IContextReplayerFake(SegmentCtxFake* segmentCtx)
: segmentCtx(segmentCtx)
{
    ON_CALL(*this, ReplayStripeFlushed).WillByDefault(::testing::Invoke(this, &IContextReplayerFake::_ReplayStripeFlushed));
}

std::vector<VirtualBlkAddr>
IContextReplayerFake::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> ret(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA);
    return ret;
}

void
IContextReplayerFake::_ReplayStripeFlushed(StripeId userLsid)
{
    segmentCtx->UpdateOccupiedStripeCount(userLsid);
}
} // namespace pos
