#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/statistics/stripe_replay_status.h"

namespace pos
{
class MockStripeReplayStatus : public StripeReplayStatus
{
public:
    MockStripeReplayStatus(void)
    : StripeReplayStatus(0)
    {
    }
    MOCK_METHOD(void, Print, (), (override));
    MOCK_METHOD(void, BlockWritten, (BlkOffset startOffset, uint32_t numBlks), (override));
    MOCK_METHOD(void, StripeFlushed, (), (override));
    MOCK_METHOD(void, BlockInvalidated, (uint32_t numBlks), (override));
    MOCK_METHOD(void, SegmentAllocated, (), (override));
    MOCK_METHOD(void, StripeAllocated, (), (override));
    MOCK_METHOD(void, RecordLogFoundTime, (uint64_t time), (override));

    MOCK_METHOD(void, BlockLogFound, (BlockWriteDoneLog dat), (override));
    MOCK_METHOD(void, StripeLogFound, (StripeMapUpdatedLog dat), (override));
    MOCK_METHOD(void, GcBlockLogFound, (GcBlockMapUpdate* mapUpdate, uint32_t numBlks), (override));
    MOCK_METHOD(void, GcStripeLogFound, (GcStripeFlushedLog dat), (override));

    MOCK_METHOD(int, GetVolumeId, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(bool, IsFlushed, (), (override));
    MOCK_METHOD(StripeId, GetWbLsid, (), (override));
    MOCK_METHOD(StripeId, GetUserLsid, (), (override));
};

} // namespace pos
