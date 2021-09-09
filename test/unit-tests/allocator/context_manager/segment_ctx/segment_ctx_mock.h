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
    MOCK_METHOD(void, BeforeFlush, (int section, char* buf), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx* ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(uint32_t, IncreaseValidBlockCount, (SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(bool, DecreaseValidBlockCount, (SegmentId segId, uint32_t cnt), (override));
    MOCK_METHOD(uint32_t, GetValidBlockCount, (SegmentId segId), (override));
    MOCK_METHOD(void, SetOccupiedStripeCount, (SegmentId segId, int count), (override));
    MOCK_METHOD(int, GetOccupiedStripeCount, (SegmentId segId), (override));
    MOCK_METHOD(bool, IncreaseOccupiedStripeCount, (SegmentId segId), (override));
    MOCK_METHOD(void, SetSegmentState, (SegmentId segId, SegmentState state, bool needlock), (override));
    MOCK_METHOD(SegmentState, GetSegmentState, (SegmentId segId, bool needlock), (override));
    MOCK_METHOD(std::mutex&, GetSegStateLock, (SegmentId segId), (override));
    MOCK_METHOD(SegmentInfo*, GetSegmentInfo, (), (override));
    MOCK_METHOD(std::mutex&, GetSegmentCtxLock, (), (override));
    MOCK_METHOD(void, CopySegmentInfoToBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
    MOCK_METHOD(void, CopySegmentInfoFromBufferforWBT, (WBTAllocatorMetaType type, char* dstBuf), (override));
};

} // namespace pos
