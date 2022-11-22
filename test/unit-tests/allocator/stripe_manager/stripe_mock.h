#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/allocator/stripe_manager/stripe.h"

namespace pos
{
class MockStripe : public Stripe
{
public:
    using Stripe::Stripe;
    MOCK_METHOD(void, Assign, (StripeId vsid, StripeId wbLsid, StripeId userLsid, uint32_t volumeId), (override));
    MOCK_METHOD(uint32_t, GetVolumeId, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(StripeId, GetWbLsid, (), (override));
    MOCK_METHOD(StripeId, GetUserLsid, (), (override));
    MOCK_METHOD(void, UpdateReverseMapEntry, (uint32_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (uint32_t offset), (override));
    MOCK_METHOD(int, Flush, (EventSmartPtr callback), (override));
    MOCK_METHOD(void, UpdateVictimVsa, (uint32_t offset, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVictimVsa, (uint32_t offset), (override));
    MOCK_METHOD(bool, IsFinished, (), (override));
    MOCK_METHOD(void, SetFinished, (), (override));
    MOCK_METHOD(uint32_t, GetBlksRemaining, (), (override));
    MOCK_METHOD(uint32_t, DecreseBlksRemaining, (uint32_t amount), (override));
    MOCK_METHOD(void, Refer, (), (override));
    MOCK_METHOD(void, Derefer, (uint32_t blockCount), (override));
    MOCK_METHOD(bool, IsOkToFree, (), (override));
    MOCK_METHOD(void, UpdateFlushIo, (FlushIoSmartPtr flushIo), (override));
    MOCK_METHOD(bool, IsActiveFlushTarget, (), (override));
    MOCK_METHOD(void, SetActiveFlushTarget, (), (override));
};

} // namespace pos
