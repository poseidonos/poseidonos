#include "stripemap_mock.h"

namespace pos
{
StripeMapMock::StripeMapMock(TestInfo* testInfo)
: testInfo(testInfo)
{
    map = new StripeAddr[testInfo->numUserStripes];
    for (uint32_t idx = 0; idx < testInfo->numUserStripes; idx++)
    {
        map[idx].stripeId = UNMAP_STRIPE;
    }

    ON_CALL(*this, SetLSA).WillByDefault(::testing::Invoke(this,
        &StripeMapMock::_SetLSA));
    ON_CALL(*this, GetLSA).WillByDefault(::testing::Invoke(this,
        &StripeMapMock::_GetLSA));
}

StripeMapMock::~StripeMapMock(void)
{
    delete [] map;
}

MpageList
StripeMapMock::GetDirtyStripeMapPages(int vsid)
{
    int numEntriesPerPage = testInfo->metaPageSize / 4;

    MpageList dirty;
    MpageNum pageNum = vsid / numEntriesPerPage;
    dirty.insert(pageNum);

    return dirty;
}

StripeAddr
StripeMapMock::_GetLSA(StripeId vsid)
{
    assert(vsid < testInfo->numUserStripes);
    return map[vsid];
}

int
StripeMapMock::_SetLSA(StripeId vsid, StripeId lsid, StripeLoc loc)
{
    assert(vsid < testInfo->numUserStripes);

    StripeAddr entry = {.stripeLoc = loc, .stripeId = lsid};
    map[vsid] = entry;
    return 0;
}

LsidRefResult
StripeMapMock::GetLSAandReferLsid(StripeId vsid)
{
    LsidRefResult result;
    return result;
}

StripeId
StripeMapMock::GetRandomLsid(StripeId vsid)
{
    return UNMAP_STRIPE;
}

bool
StripeMapMock::IsInUserDataArea(StripeAddr entry)
{
    return (entry.stripeLoc == IN_USER_AREA);
}

bool
StripeMapMock::IsInWriteBufferArea(StripeAddr entry)
{
    return (entry.stripeLoc == IN_WRITE_BUFFER_AREA);
}
} // namespace pos
