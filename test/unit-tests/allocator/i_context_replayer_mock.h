#include <gmock/gmock.h>

#include <list>
#include <string>
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
    MOCK_METHOD(void, ReplayStripeAllocation, (StripeId vsid, StripeId wbLsid), (override));
    MOCK_METHOD(void, ReplayStripeFlushed, (StripeId wbLsid), (override));
    MOCK_METHOD(void, ResetActiveStripeTail, (int index), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(void, ResetSegmentsStates, (), (override));
};

} // namespace pos
