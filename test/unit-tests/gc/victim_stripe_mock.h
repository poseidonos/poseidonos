#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/gc/victim_stripe.h"

namespace pos
{
class MockBlkInfo : public BlkInfo
{
public:
    using BlkInfo::BlkInfo;
};

class MockVictimStripe : public VictimStripe
{
public:
    using VictimStripe::VictimStripe;
    MOCK_METHOD(void, Load, (StripeId, CallbackSmartPtr), (override));
    MOCK_METHOD(list<BlkInfo>&, GetBlkInfoList, (uint32_t), (override));
    MOCK_METHOD(uint32_t, GetBlkInfoListSize, (), (override));
    MOCK_METHOD(bool, LoadValidBlock, (), (override));
};

} // namespace pos
