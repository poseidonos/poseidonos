#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_wbstripe_ctx.h"

namespace pos
{
class MockIWBStripeCtx : public IWBStripeCtx
{
public:
    using IWBStripeCtx::IWBStripeCtx;
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId vsid, StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId wbLsid), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));
};

} // namespace pos
