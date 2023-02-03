#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/i_context_replayer.h"
using ::testing::Mock;

namespace pos
{
class IContextReplayerMock : public IContextReplayer, Mock
{
public:
    using IContextReplayer::IContextReplayer;
    MOCK_METHOD(void, ResetDirtyContextVersion, (int owner), (override));
    MOCK_METHOD(void, ReplaySsdLsid, (StripeId currentSsdLsid), (override));
    MOCK_METHOD(void, ReplaySegmentAllocation, (StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId wbLsid, StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeRelease, (StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId userLsid), (override));
    MOCK_METHOD(void, SetActiveStripeTail,
        (int index, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));
    virtual void ResetSegmentsStates(void) {}
    virtual std::vector<VirtualBlkAddr>
    GetAllActiveStripeTail(void)
    {
        std::vector<VirtualBlkAddr> ret(ACTIVE_STRIPE_TAIL_ARRAYLEN, UNMAP_VSA);
        return ret;
    }
};

} // namespace pos
