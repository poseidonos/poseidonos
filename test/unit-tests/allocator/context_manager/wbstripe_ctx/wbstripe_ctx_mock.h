#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/context_manager/wbstripe_ctx/wbstripe_ctx.h"

namespace pos
{
class MockWbStripeCtx : public WbStripeCtx
{
public:
    using WbStripeCtx::WbStripeCtx;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(void, AfterLoad, (char* buf), (override));
    MOCK_METHOD(void, BeforeFlush, (int section, char* buf), (override));
    MOCK_METHOD(void, FinalizeIo, (AsyncMetaFileIoCtx * ctx), (override));
    MOCK_METHOD(char*, GetSectionAddr, (int section), (override));
    MOCK_METHOD(int, GetSectionSize, (int section), (override));
    MOCK_METHOD(uint64_t, GetStoredVersion, (), (override));
    MOCK_METHOD(void, ResetDirtyVersion, (), (override));
    MOCK_METHOD(std::vector<VirtualBlkAddr>, GetAllActiveStripeTail, (), (override));
    MOCK_METHOD(void, AllocWbStripe, (StripeId stripeId), (override));
    MOCK_METHOD(StripeId, AllocFreeWbStripe, (), (override));
    MOCK_METHOD(void, ReleaseWbStripe, (StripeId stripeId), (override));
    MOCK_METHOD(void, SetAllocatedWbStripeCount, (int count), (override));
    MOCK_METHOD(uint64_t, GetAllocatedWbStripeCount, (), (override));
    MOCK_METHOD(uint64_t, GetNumTotalWbStripe, (), (override));
    MOCK_METHOD(VirtualBlkAddr, GetActiveStripeTail, (ASTailArrayIdx asTailArrayIdx), (override));
    MOCK_METHOD(void, SetActiveStripeTail, (ASTailArrayIdx asTailArrayIdx, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(std::mutex&, GetActiveStripeTailLock, (ASTailArrayIdx asTailArrayIdx), (override));
    MOCK_METHOD(std::mutex&, GetAllocWbLsidBitmapLock, (), (override));
};

} // namespace pos
