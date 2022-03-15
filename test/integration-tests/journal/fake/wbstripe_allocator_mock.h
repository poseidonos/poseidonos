#pragma once

#include <map>

#include "gmock/gmock.h"
#include "src/allocator/i_wbstripe_allocator.h"

namespace pos
{
class WBStripeAllocatorMock : public IWBStripeAllocator
{
public:
    WBStripeAllocatorMock(void) {}
    virtual ~WBStripeAllocatorMock(void) {}

    MOCK_METHOD(int, ReconstructActiveStripe,
        (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe,
        (StripeId wbLsid, VirtualBlkAddr tail), (override));

    virtual Stripe* GetStripe(StripeAddr& lsa) override { return nullptr; }
    virtual Stripe* GetStripe(StripeId wbLsid) override { return nullptr; }
    virtual void FreeWBStripeId(StripeId lsid) override {}

    virtual bool ReferLsidCnt(StripeAddr& lsa) override { return true; }
    virtual void DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount) override {}

    virtual void FlushActiveStripes(uint32_t volumeId) {}
    virtual bool FinalizeActiveStripes(int volumeId) {}
    virtual void GetWbStripes(FlushIoSmartPtr flushIo) {}

    virtual void FlushAllActiveStripes(void) override {}
    virtual int FlushPendingActiveStripes(void) { return 0; }
};

} // namespace pos
