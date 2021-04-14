#pragma once

#include <vector>

#include "src/allocator/context_manager/active_stripe_index_info.h"
#include "src/allocator/i_wbstripe_ctx.h"

namespace pos
{
class WBStripeCtxMock : public IWBStripeCtx
{
public:
    WBStripeCtxMock(void) {}
    virtual ~WBStripeCtxMock(void) {}

    MOCK_METHOD(void, ReplayStripeAllocation,
        (StripeId vsid, StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed,
        (StripeId wbLsid), (override));
    MOCK_METHOD(void, ResetActiveStripeTail,
        (int index), (override));

    virtual std::vector<VirtualBlkAddr>
    GetAllActiveStripeTail(void)
    {
        std::vector<VirtualBlkAddr> ret(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA);
        return ret;
    }
};

} // namespace pos
