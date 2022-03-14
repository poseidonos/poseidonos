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
    MOCK_METHOD(Stripe*, GetStripe, (StripeAddr& lsidEntry), (override));
    MOCK_METHOD(Stripe*, GetStripe, (StripeId wbLsid), (override));
    MOCK_METHOD(void, FreeWBStripeId, (StripeId lsid), (override));
    MOCK_METHOD(void, FlushActiveStripes, (uint32_t volumeId), (override));
    MOCK_METHOD(void, GetWbStripes, (FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(bool, ReferLsidCnt, (StripeAddr& lsa), (override));
    MOCK_METHOD(void, DereferLsidCnt, (StripeAddr& lsa, uint32_t blockCount), (override));
    MOCK_METHOD(int, ReconstructActiveStripe, (uint32_t volumeId, StripeId wbLsid, VirtualBlkAddr tailVsa, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(Stripe*, FinishReconstructedStripe, (StripeId wbLsid, VirtualBlkAddr tail), (override));
    MOCK_METHOD(void, SetActiveStripeTail, (uint32_t volumeId, VirtualBlkAddr tail, StripeId wbLsid), (override));
    MOCK_METHOD(void, FlushAllActiveStripes, (), (override));
    MOCK_METHOD(bool, FinalizeActiveStripes, (int volumeId), (override));
    MOCK_METHOD(int, FlushPendingActiveStripes, (), (override));
    MOCK_METHOD(int, FlushOnlineStripesInSegment, (std::set<SegmentId>& segments), (override));
    MOCK_METHOD(void, FinalizeWriteIO, (std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone), (override));
    MOCK_METHOD(int, CheckAllActiveStripes, (std::vector<Stripe*>& stripesToFlush, std::vector<StripeId>& vsidToCheckFlushDone), (override));
    MOCK_METHOD(void, PushStripeToStripeArray, (Stripe* stripe), (override));
    MOCK_METHOD(int, _RequestStripeFlush, (Stripe* stripe), (override));
};

} // namespace pos
