#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/i_context_replayer.h"

namespace pos
{
class MockIContextReplayer : public IContextReplayer
{
public:
    using IContextReplayer::IContextReplayer;
    MOCK_METHOD(void, ResetDirtyContextVersion, (int owner), (override));
    MOCK_METHOD(void, ReplaySsdLsid, (StripeId currentSsdLsid), (override));
    MOCK_METHOD(void, ReplaySegmentAllocation, (StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId wbLsid, StripeId userLsid), (override));
    MOCK_METHOD(void, ReplayStripeRelease, (StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId userLsid), (override));
    MOCK_METHOD(void, SetActiveStripeTail, (int index, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(void, ResetSegmentsStates, (), (override));
};

} // namespace pos
