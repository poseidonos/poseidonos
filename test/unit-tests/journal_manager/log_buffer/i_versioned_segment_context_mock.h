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
    MOCK_METHOD(void, Init, (JournalConfiguration * journalConfiguration, SegmentInfoData* loadedSegmentInfos, uint32_t numSegments), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, DecreaseValidBlockCount, (int logGroupId, SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(void, IncreaseOccupiedStripeCount, (int logGroupId, SegmentId segId), (override));
    MOCK_METHOD(int, GetNumSegments, (), (override));
    MOCK_METHOD(int, GetNumLogGroups, (), (override));
    MOCK_METHOD(SegmentInfoData*, GetUpdatedInfoDataToFlush, (int logGroupId), (override));
    MOCK_METHOD(void, Init, (JournalConfiguration* journalConfiguration, SegmentInfoData* loadedSegmentInfo, uint32_t numSegments,
        std::vector<std::shared_ptr<VersionedSegmentInfo>> inputVersionedSegmentInfo), (override));
    MOCK_METHOD(void, LogFilled, (int logGroupId, const MapList& dirty), (override));
    MOCK_METHOD(void, LogBufferReseted, (int logGroupId), (override));
    MOCK_METHOD(void, NotifySegmentFreed, (SegmentId segmentId), (override));
};

} // namespace pos
