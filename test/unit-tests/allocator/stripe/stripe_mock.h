#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/stripe/stripe.h"

namespace pos
{
class MockStripe : public Stripe
{
public:
    using Stripe::Stripe;
    MOCK_METHOD(void, Assign, (StripeId vsid, StripeId lsid, ASTailArrayIdx tailarrayidx), (override));
    MOCK_METHOD(uint32_t, GetVolumeId, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(void, SetVsid, (StripeId virtsid), (override));
    MOCK_METHOD(StripeId, GetWbLsid, (), (override));
    MOCK_METHOD(void, SetWbLsid, (StripeId wbAreaLsid), (override));
    MOCK_METHOD(StripeId, GetUserLsid, (), (override));
    MOCK_METHOD(void, SetUserLsid, (StripeId userAreaLsid), (override));
    MOCK_METHOD(void, UpdateReverseMapEntry, (uint32_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (uint32_t offset), (override));
    MOCK_METHOD(int, Flush, (EventSmartPtr callback), (override));
    MOCK_METHOD(void, UpdateVictimVsa, (uint32_t offset, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVictimVsa, (uint32_t offset), (override));
    MOCK_METHOD(bool, IsFinished, (), (override));
    MOCK_METHOD(void, SetFinished, (bool state), (override));
    MOCK_METHOD(uint32_t, GetBlksRemaining, (), (override));
    MOCK_METHOD(uint32_t, DecreseBlksRemaining, (uint32_t amount), (override));
    MOCK_METHOD(void, Refer, (), (override));
    MOCK_METHOD(void, Derefer, (uint32_t blockCount), (override));
    MOCK_METHOD(bool, IsOkToFree, (), (override));
    MOCK_METHOD(void, AddDataBuffer, (void* buf), (override));
    MOCK_METHOD(DataBufferIter, DataBufferBegin, (), (override));
    MOCK_METHOD(DataBufferIter, DataBufferEnd, (), (override));
    MOCK_METHOD(void, UpdateFlushIo, (FlushIoSmartPtr flushIo), (override));
};

} // namespace pos
