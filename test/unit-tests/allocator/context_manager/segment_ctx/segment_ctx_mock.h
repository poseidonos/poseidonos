#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{
class MockSegmentCtx : public SegmentCtx
{
public:
    using SegmentCtx::SegmentCtx;
    MOCK_METHOD(void, Init, (EventScheduler* eventScheduler_), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (char* buf, ContextSectionBuffer externalBuf), (override));
    MOCK_METHOD(void, AfterFlush, (char* buf), (override));
    MOCK_METHOD(ContextSectionAddr, GetSectionInfo, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
    MOCK_METHOD(uint64_t, GetTotalDataSize, (), (override));
    MOCK_METHOD(bool, MoveToFreeState, (SegmentId segId), (override));
    MOCK_METHOD(int, GetValidBlockCount, (SegmentId segId), (override));
    MOCK_METHOD(int, GetOccupiedStripeCount, (SegmentId segId), (override));
    MOCK_METHOD(SegmentState, GetSegmentState, (SegmentId segId), (override));
    MOCK_METHOD(void, ResetSegmentsStates, (), (override));
    MOCK_METHOD(void, AllocateSegment, (SegmentId segId), (override));
    MOCK_METHOD(SegmentId, AllocateFreeSegment, (), (override));
    MOCK_METHOD(uint64_t, GetNumOfFreeSegment, (), (override));
    MOCK_METHOD(uint64_t, GetNumOfFreeSegmentWoLock, (), (override));
    MOCK_METHOD(int, GetAllocatedSegmentCount, (), (override));
    MOCK_METHOD(SegmentId, AllocateGCVictimSegment, (), (override));
    MOCK_METHOD(SegmentId, GetRebuildTargetSegment, (), (override));
    MOCK_METHOD(int, SetRebuildCompleted, (SegmentId segId), (override));
    MOCK_METHOD(int, MakeRebuildTarget, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetNvramSegmentList, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetVictimSegmentList, (), (override));
    MOCK_METHOD(uint32_t, GetVictimSegmentCount, (), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetRebuildSegmentList, (), (override));
    MOCK_METHOD(bool, LoadRebuildList, (), (override));
    MOCK_METHOD(void, CopySegmentInfoToBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, CopySegmentInfoFromBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, AddSegmentFreeSubscriber, (ISegmentFreeSubscriber * subscriber), (override));
    MOCK_METHOD(SegmentInfoData*, GetSegmentInfoDataArray, (), (override));
};

} // namespace pos
