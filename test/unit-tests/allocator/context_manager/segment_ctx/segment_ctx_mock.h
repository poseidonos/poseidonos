#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"

namespace pos
{
class MockSegmentCtx : public SegmentCtx
{
public:
    using SegmentCtx::SegmentCtx;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (char* buf), (override));
    MOCK_METHOD(std::mutex&, GetCtxLock, (), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx* ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(std::string, GetFilename, (), (override));
    MOCK_METHOD(uint32_t, GetSignature, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
    MOCK_METHOD(void, IncreaseValidBlockCount, (SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(bool, DecreaseValidBlockCount, (SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(uint32_t, GetValidBlockCount, (SegmentId segId), (override));
    MOCK_METHOD(int, GetOccupiedStripeCount, (SegmentId segId), (override));
    MOCK_METHOD(bool, IncreaseOccupiedStripeCount, (SegmentId segId), (override));
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
    MOCK_METHOD(int, MakeRebuildTarget, (std::set<SegmentId>& segmentList), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetRebuildSegmentList, (), (override));
    MOCK_METHOD(bool, LoadRebuildList, (), (override));
    MOCK_METHOD(void, CopySegmentInfoToBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, CopySegmentInfoFromBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, UpdateGcFreeSegment, (uint32_t arrayId), (override));
};

} // namespace pos
