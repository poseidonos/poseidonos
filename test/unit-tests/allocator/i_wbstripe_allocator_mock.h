#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/i_wbstripe_allocator.h"

namespace pos
{
class MockIWBStripeAllocator : public IWBStripeAllocator
{
public:
    using IWBStripeAllocator::IWBStripeAllocator;
    MOCK_METHOD(Stripe*, GetStripe, (StripeAddr& lsa), (override));
    MOCK_METHOD(void, FreeWBStripeId, (StripeId lsid), (override));
    MOCK_METHOD(bool, ReferLsidCnt, (StripeAddr& lsa), (override));
    MOCK_METHOD(void, DereferLsidCnt, (StripeAddr& lsa, uint32_t blockCount), (override));
    MOCK_METHOD(void, FlushActiveStripes, (uint32_t volumeId), (override));
    MOCK_METHOD(void, GetWbStripes, (FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(void, FlushAllActiveStripes, (), (override));
    MOCK_METHOD(bool, FinalizeActiveStripes, (int volumeId), (override));
    MOCK_METHOD(int, ReconstructActiveStripe, (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(int, FlushPendingActiveStripes, (), (override));
};

} // namespace pos
