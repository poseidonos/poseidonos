#pragma once

#include "gmock/gmock.h"

#include "src/include/address_type.h"
#include "src/mapper/i_stripemap.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
class StripeMapMock : public IStripeMap
{
public:
    explicit StripeMapMock(TestInfo* testInfo);
    virtual ~StripeMapMock(void);

    virtual MpageList GetDirtyStripeMapPages(int vsid) override;

    MOCK_METHOD(StripeAddr, GetLSA, (StripeId vsid), (override));
    MOCK_METHOD(int, SetLSA, (StripeId vsid, StripeId lsid, StripeLoc loc), (override));

    virtual LsidRefResult GetLSAandReferLsid(StripeId vsid) override;
    virtual StripeId GetRandomLsid(StripeId vsid) override;

    virtual bool IsInUserDataArea(StripeAddr entry) override;
    virtual bool IsInWriteBufferArea(StripeAddr entry) override;

private:
    StripeAddr _GetLSA(StripeId vsid);
    int _SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc);

    TestInfo* testInfo;
    StripeAddr* map;
};
} // namespace pos
