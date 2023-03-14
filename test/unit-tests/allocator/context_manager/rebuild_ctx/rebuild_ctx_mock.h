#include <gmock/gmock.h>
#include <set>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"

namespace pos
{
class MockRebuildCtx : public RebuildCtx
{
public:
    using RebuildCtx::RebuildCtx;
    MOCK_METHOD(void, SetAllocatorFileIo, (AllocatorFileIo* fileIo_), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (char* buf, ContextSectionBuffer externalBuf), (override));
    MOCK_METHOD(void, AfterFlush, (char* buf), (override));
    MOCK_METHOD(ContextSectionAddr, GetSectionInfo, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(int, GetNumSections, (), (override));
    MOCK_METHOD(int, FlushRebuildSegmentList, (std::set<SegmentId> segIdSet), (override));
    MOCK_METHOD(std::set<SegmentId>, GetList, (), (override));
};

} // namespace pos
