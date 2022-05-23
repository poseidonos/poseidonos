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
    MOCK_METHOD(void, FinishStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(StripeId, GetUserStripeId, (StripeId vsid), (override));
    MOCK_METHOD(int, LoadPendingStripesToWriteBuffer, (), (override));

    virtual Stripe* GetStripe(StripeId wbLsid) override { return nullptr; }
    virtual void FreeWBStripeId(StripeId lsid) override {}

    virtual bool ReferLsidCnt(StripeAddr& lsa) override { return true; }
    virtual void DereferLsidCnt(StripeAddr& lsa, uint32_t blockCount) override {}

    virtual int FlushAllPendingStripes(void) { return 0; }
    virtual int FlushAllPendingStripesInVolume(int volumeId) { return 0; }
    virtual int FlushAllPendingStripesInVolume(int volumeId, FlushIoSmartPtr flushIo) { return 0; }
};

} // namespace pos
