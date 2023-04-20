#include "i_context_replayer_fake.h"

#include "test/integration-tests/journal/fake/segment_ctx_fake.h"

namespace pos
{
using ::testing::_;
using ::testing::AtLeast;

IContextReplayerFake::IContextReplayerFake(SegmentCtxFake* segmentCtx)
: segmentCtx(segmentCtx)
{
    ON_CALL(*this, ReplayStripeFlushed).WillByDefault(::testing::Invoke(this, &IContextReplayerFake::_ReplayStripeFlushed));
    ON_CALL(*this, ReplayBlockValidated).WillByDefault(::testing::Invoke(this, &IContextReplayerFake::_ReplayBlockValidated));
    EXPECT_CALL(*this, ReplayBlockValidated).Times(AtLeast(0));
    ON_CALL(*this, ReplayBlockInvalidated).WillByDefault(::testing::Invoke(this, &IContextReplayerFake::_ReplayBlockInvalidated));
    EXPECT_CALL(*this, ReplayBlockInvalidated).Times(AtLeast(0));
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
    segmentCtx->ReplayStripeFlushed(userLsid);
}

void
IContextReplayerFake::_ReplayBlockValidated(VirtualBlks blks)
{
    segmentCtx->ValidateBlks(blks);
}

void
IContextReplayerFake::_ReplayBlockInvalidated(VirtualBlks blks, bool allowVictimSegRelease)
{
    segmentCtx->ReplayBlockInvalidated(blks, allowVictimSegRelease);
}

} // namespace pos
