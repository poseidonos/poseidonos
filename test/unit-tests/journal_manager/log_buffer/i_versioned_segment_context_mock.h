#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"

namespace pos
{
class MockIVersionedSegmentContext : public IVersionedSegmentContext
{
public:
    using IVersionedSegmentContext::IVersionedSegmentContext;
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, SegmentInfo* loadedSegmentInfos, int numSegments), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, IncreaseOccupiedStripeCount, (int logGroupId, SegmentId segId), (override));
};

} // namespace pos
