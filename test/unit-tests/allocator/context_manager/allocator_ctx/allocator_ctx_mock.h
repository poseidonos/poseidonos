#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

namespace pos
{
class MockAllocatorCtx : public AllocatorCtx
{
public:
    using AllocatorCtx::AllocatorCtx;
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
    MOCK_METHOD(void, SetCurrentSsdLsid, (StripeId stripe), (override));
    MOCK_METHOD(StripeId, GetCurrentSsdLsid, (), (override));
    MOCK_METHOD(void, SetNextSsdLsid, (SegmentId segId), (override));
    MOCK_METHOD(void, AllocWbStripe, (StripeId stripeId), (override));
    MOCK_METHOD(StripeId, AllocFreeWbStripe, (), (override));
    MOCK_METHOD(void, ReleaseWbStripe, (StripeId stripeId), (override));
    MOCK_METHOD(void, SetAllocatedWbStripeCount, (int count), (override));
    MOCK_METHOD(uint64_t, GetAllocatedWbStripeCount, (), (override));
    MOCK_METHOD(uint64_t, GetNumTotalWbStripe, (), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(VirtualBlkAddr, GetActiveStripeTail, (ASTailArrayIdx asTailArrayIdx), (override));
    MOCK_METHOD(void, SetActiveStripeTail, (ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(std::mutex&, GetActiveStripeTailLock, (ASTailArrayIdx asTailArrayIdx), (override));
};

} // namespace pos
