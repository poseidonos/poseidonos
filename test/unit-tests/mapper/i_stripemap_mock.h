#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_stripemap.h"

namespace pos
{
class MockIStripeMap : public IStripeMap
{
public:
    using IStripeMap::IStripeMap;
    MOCK_METHOD(StripeAddr, GetLSA, (StripeId vsid), (override));
    MOCK_METHOD(LsidRefResult, GetLSAandReferLsid, (StripeId vsid), (override));
    MOCK_METHOD(StripeId, GetRandomLsid, (StripeId vsid), (override));
    MOCK_METHOD(int, SetLSA, (StripeId vsid, StripeId lsid, StripeLoc loc), (override));
    MOCK_METHOD(bool, IsInUserDataArea, (StripeAddr entry), (override));
    MOCK_METHOD(bool, IsInWriteBufferArea, (StripeAddr entry), (override));
    MOCK_METHOD(MpageList, GetDirtyStripeMapPages, (int vsid), (override));
};

} // namespace pos
