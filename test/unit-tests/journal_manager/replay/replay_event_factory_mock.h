#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/replay/replay_event_factory.h"

namespace pos
{
class MockReplayEventFactory : public ReplayEventFactory
{
public:
    using ReplayEventFactory::ReplayEventFactory;
    MOCK_METHOD(ReplayEvent*, CreateBlockWriteReplayEvent, (int volId,
            BlkAddr startRba, VirtualBlkAddr startVsa, uint64_t numBlks, bool replaySegmentInfo), (override));
    MOCK_METHOD(ReplayEvent*, CreateStripeMapUpdateReplayEvent, (StripeId vsid, StripeAddr dest), (override));
    MOCK_METHOD(ReplayEvent*, CreateStripeFlushReplayEvent, (StripeId vsid, StripeId wbLsid, StripeId userLsid), (override));
    MOCK_METHOD(ReplayEvent*, CreateStripeAllocationReplayEvent, (StripeId vsid, StripeId wbLsid), (override));
    MOCK_METHOD(ReplayEvent*, CreateSegmentAllocationReplayEvent, (StripeId userLsid), (override));
};

} // namespace pos
