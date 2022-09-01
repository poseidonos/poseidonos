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
    MOCK_METHOD(int, FlushRebuildSegmentList, (std::set<SegmentId> segIdSet), (override));
    MOCK_METHOD(std::set<SegmentId>, GetList, (), (override));
};

} // namespace pos
