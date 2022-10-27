#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/gc_ctx/gc_ctx_mock.h"
#include "test/unit-tests/allocator/context_manager/rebuild_ctx/rebuild_ctx_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using testing::NiceMock;

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
    MOCK_METHOD(void, MoveToFreeState, (SegmentId segId), (override));
    MOCK_METHOD(uint32_t, GetValidBlockCount, (SegmentId segId), (override));
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
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(uint32_t, GetRebuildTargetSegmentCount, (), (override));
    MOCK_METHOD(std::set<SegmentId>, GetRebuildSegmentList, (), (override));
    MOCK_METHOD(bool, LoadRebuildList, (), (override));
    MOCK_METHOD(void, CopySegmentInfoToBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, CopySegmentInfoFromBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, ValidateBlks, (VirtualBlks blks), (override));
    MOCK_METHOD(bool, InvalidateBlks, (VirtualBlks blks, bool allowVictimSegRelease), (override));
    MOCK_METHOD(bool, UpdateOccupiedStripeCount, (StripeId lsid), (override));
    MOCK_METHOD(void, ValidateBlocksWithGroupId, (VirtualBlks blks, int logGroupId), (override));
    MOCK_METHOD(bool, InvalidateBlocksWithGroupId, (VirtualBlks blks, bool isForced, int logGroupId), (override));
    MOCK_METHOD(bool, UpdateStripeCount, (StripeId lsid, int logGroupId), (override));
};
} // namespace pos
