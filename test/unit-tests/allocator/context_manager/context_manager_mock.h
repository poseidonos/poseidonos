#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/context_manager.h"
#include "src/journal_manager/log_buffer/versioned_segment_ctx.h"

namespace pos
{
class MockContextManager : public ContextManager
{
public:
    using ContextManager::ContextManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, FlushContexts, (EventSmartPtr callback, bool sync, int logGroupId), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(int, MakeRebuildTargetSegmentList, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetNvramSegmentList, (), (override));
    MOCK_METHOD(int, GetGcThreshold, (GcMode mode), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(int, SetNextSsdLsid, (), (override));
    MOCK_METHOD(char*, GetContextSectionAddr, (int owner, int section), (override));
    MOCK_METHOD(int, GetContextSectionSize, (int owner, int section), (override));
    MOCK_METHOD(RebuildCtx*, GetRebuildCtx, (), (override));
    MOCK_METHOD(SegmentCtx*, GetSegmentCtx, (), (override));
    MOCK_METHOD(ISegmentCtx*, GetISegmentCtx, (), (override));
    MOCK_METHOD(AllocatorCtx*, GetAllocatorCtx, (), (override));
    MOCK_METHOD(ContextReplayer*, GetContextReplayer, (), (override));
    MOCK_METHOD(GcCtx*, GetGcCtx, (), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
    MOCK_METHOD(BlockAllocationStatus*, GetAllocationStatus, (), (override));
    MOCK_METHOD(void, PrepareVersionedSegmentCtx, (IVersionedSegmentContext* versionedSegCtx), (override));
    MOCK_METHOD(void, ResetFlushedInfo, (int logGroupId), (override));
    MOCK_METHOD(void, SetAllocateDuplicatedFlush, (bool flag), (override));
};

} // namespace pos
