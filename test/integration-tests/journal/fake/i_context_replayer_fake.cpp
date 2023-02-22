#include "i_context_replayer_fake.h"

#include "src/allocator/i_segment_ctx.h"

namespace pos
{
std::vector<VirtualBlkAddr>
IContextReplayerFake::GetAllActiveStripeTail(void)
{
    std::vector<VirtualBlkAddr> ret(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA);
    return ret;
}
} // namespace pos
