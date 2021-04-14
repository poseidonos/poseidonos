#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/wb_stripe_manager/wbstripe_manager.h"

namespace pos
{
class MockWBStripeManager : public WBStripeManager
{
public:
    using WBStripeManager::WBStripeManager;
    MOCK_METHOD(Stripe*, GetStripe, (StripeAddr & lsidEntry), (override));
    MOCK_METHOD(StripeId, AllocateUserDataStripeId, (StripeId vsid), (override));
    MOCK_METHOD(void, FreeWBStripeId, (StripeId lsid), (override));
    MOCK_METHOD(void, GetAllActiveStripes, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, WaitPendingWritesOnStripes, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, WaitStripesFlushCompletion, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, ReferLsidCnt, (StripeAddr & lsa), (override));
    MOCK_METHOD(void, DereferLsidCnt, (StripeAddr & lsa, uint32_t blockCount), (override));
    MOCK_METHOD(void, FlushAllActiveStripes, (), (override));
    MOCK_METHOD(int, ReconstructActiveStripe, (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, ASTailArrayIdx tailarrayidx), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(int, RestoreActiveStripeTail, (int index, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(int, FlushPendingActiveStripes, (), (override));
    MOCK_METHOD(int, PrepareRebuild, (), (override));
    MOCK_METHOD(int, StopRebuilding, (), (override));
    MOCK_METHOD(Stripe*, GetStripe, (StripeId wbLsid), (override));
};

} // namespace pos
