#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/context_manager.h"

namespace pos
{
class MockContextManager : public ContextManager
{
public:
    using ContextManager::ContextManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(int, FlushContextsSync, (), (override));
    MOCK_METHOD(int, FlushContextsAsync, (EventSmartPtr callback), (override));
    MOCK_METHOD(void, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (bool forUser), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, AllocateRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, ReleaseRebuildSegment, (SegmentId segmentId), (override));
    MOCK_METHOD(bool, NeedRebuildAgain, (), (override));
    MOCK_METHOD(int, GetNumOfFreeSegment, (bool needLock), (override));
    MOCK_METHOD(CurrentGcMode, GetCurrentGcMode, (), (override));
    MOCK_METHOD(int, GetGcThreshold, (CurrentGcMode mode), (override));
    MOCK_METHOD(uint64_t, GetStoredContextVersion, (int owner), (override));
    MOCK_METHOD(void, FreeUserDataSegment, (SegmentId segId), (override));
    MOCK_METHOD(int, SetNextSsdLsid, (), (override));
    MOCK_METHOD(char*, GetContextSectionAddr, (int owner, int section), (override));
    MOCK_METHOD(int, GetContextSectionSize, (int owner, int section), (override));
    MOCK_METHOD(RebuildCtx*, GetRebuildCtx, (), (override));
    MOCK_METHOD(SegmentCtx*, GetSegmentCtx, (), (override));
    MOCK_METHOD(AllocatorCtx*, GetAllocatorCtx, (), (override));
    MOCK_METHOD(WbStripeCtx*, GetWbStripeCtx, (), (override));
    MOCK_METHOD(ContextReplayer*, GetContextReplayer, (), (override));
    MOCK_METHOD(GcCtx*, GetGcCtx, (), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
};

} // namespace pos
