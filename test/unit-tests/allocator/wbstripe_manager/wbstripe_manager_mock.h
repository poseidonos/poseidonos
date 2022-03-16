#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/wbstripe_manager/wbstripe_manager.h"

namespace pos
{
class MockWBStripeManager : public WBStripeManager
{
public:
    using WBStripeManager::WBStripeManager;
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(Stripe*, GetStripe, (StripeId wbLsid), (override));
    MOCK_METHOD(void, FreeWBStripeId, (StripeId lsid), (override));
    MOCK_METHOD(bool, ReferLsidCnt, (StripeAddr& lsa), (override));
    MOCK_METHOD(void, DereferLsidCnt, (StripeAddr& lsa, uint32_t blockCount), (override));
    MOCK_METHOD(int, ReconstructActiveStripe, (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(void, FinishStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(int, FlushAllPendingStripes, (), (override));
    MOCK_METHOD(int, FlushAllPendingStripesInVolume, (int volumeId), (override));
    MOCK_METHOD(int, FlushAllPendingStripesInVolume, (int volumeId, FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(int, FlushAllWbStripes, (), (override));
    MOCK_METHOD(void, PushStripeToStripeArray, (Stripe* stripe), (override));
    MOCK_METHOD(int, _RequestStripeFlush, (Stripe* stripe), (override));
};

} // namespace pos
