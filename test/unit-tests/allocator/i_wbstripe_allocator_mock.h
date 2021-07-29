#include <gmock/gmock.h>

#include <list>
#include <map>
#include <string>
#include <vector>

#include "src/allocator/i_wbstripe_allocator.h"

namespace pos
{
class MockIWBStripeAllocator : public IWBStripeAllocator
{
public:
    using IWBStripeAllocator::IWBStripeAllocator;
    MOCK_METHOD(Stripe*, GetStripe, (StripeAddr & lsa), (override));
    MOCK_METHOD(StripeId, AllocateUserDataStripeId, (StripeId vsid), (override));
    MOCK_METHOD(void, FreeWBStripeId, (StripeId lsid), (override));
    MOCK_METHOD(bool, ReferLsidCnt, (StripeAddr & lsa), (override));
    MOCK_METHOD(void, DereferLsidCnt, (StripeAddr & lsa, uint32_t blockCount), (override));
    MOCK_METHOD(void, FlushActiveStripes, (uint32_t volumeId), (override));
    MOCK_METHOD(bool, WaitStripesFlushCompletion, (uint32_t volumeId), (override));
    MOCK_METHOD(void, FlushAllActiveStripes, (), (override));
    MOCK_METHOD(int, ReconstructActiveStripe, (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(int, RestoreActiveStripeTail, (uint32_t volumeId, VirtualBlkAddr tail, StripeId wbLsid, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(int, FlushPendingActiveStripes, (), (override));
    MOCK_METHOD(int, PrepareRebuild, (), (override));
};

} // namespace pos
