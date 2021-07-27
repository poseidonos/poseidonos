#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/statistics/stripe_info.h"

namespace pos
{
class MockStripeInfo : public StripeInfo
{
public:
    using StripeInfo::StripeInfo;
    MOCK_METHOD(int, GetVolumeId, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
    MOCK_METHOD(StripeId, GetWbLsid, (), (override));
    MOCK_METHOD(StripeId, GetUserLsid, (), (override));
    MOCK_METHOD(BlkOffset, GetLastOffset, (), (override));
    MOCK_METHOD(bool, IsLastOffsetValid, (), (override));
    MOCK_METHOD(int, GetWbIndex, (), (override));
    MOCK_METHOD(bool, IsWbIndexValid, (), (override));
};

} // namespace pos
