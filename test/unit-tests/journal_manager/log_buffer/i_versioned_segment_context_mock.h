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
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, SegmentInfo* loadedSegmentInfos, uint32_t numSegments), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, IncreaseOccupiedStripeCount, (int logGroupId, SegmentId segId), (override));
    MOCK_METHOD(int, GetNumSegments, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(SegmentInfo*, GetUpdatedInfoToFlush, (int logGroupId), (override));
    MOCK_METHOD(void, ResetFlushedInfo, (int logGroupId), (override));
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration, SegmentInfo* loadedSegmentInfo, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo), (override));
};

} // namespace pos
