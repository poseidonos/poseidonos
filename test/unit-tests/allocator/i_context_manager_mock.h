#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/i_context_manager.h"

namespace pos
{
class MockIContextManager : public IContextManager
{
public:
    using IContextManager::IContextManager;
    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, int logGroupId), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(int, GetGcThreshold, (GcMode mode), (override));
    MOCK_METHOD(SegmentCtx*, GetSegmentCtx, (), (override));
    MOCK_METHOD(GcCtx*, GetGcCtx, (), (override));
    MOCK_METHOD(void, PrepareVersionedSegmentCtx, (IVersionedSegmentContext* versionedSegCtx), (override));
    MOCK_METHOD(void, ResetFlushedInfo, (int logGroupId), (override));
    MOCK_METHOD(void, SetAllocateDuplicatedFlush, (bool flag), (override));
};

} // namespace pos
