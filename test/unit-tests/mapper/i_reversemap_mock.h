#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_reversemap.h"

namespace pos
{
class MockIReverseMap : public IReverseMap
{
public:
    using IReverseMap::IReverseMap;
    MOCK_METHOD(int, LinkReverseMap, (Stripe * stripe, StripeId wbLsid, StripeId vsid), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (), (override));
};

} // namespace pos
