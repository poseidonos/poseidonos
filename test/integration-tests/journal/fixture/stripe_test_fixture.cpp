#include "test/integration-tests/journal/fixture/stripe_test_fixture.h"

namespace pos
{
StripeTestFixture::StripeTestFixture(void)
{
    vsid = UNMAP_STRIPE;
    volId = -1;

    wbAddr = unmapAddr;
    userAddr = unmapAddr;
}

StripeTestFixture::StripeTestFixture(int _vsid, int _volId)
{
    vsid = _vsid;
    volId = _volId;

    wbAddr.stripeId = GetWbLsid(vsid);
    wbAddr.stripeLoc = IN_WRITE_BUFFER_AREA;

    userAddr.stripeId = _vsid;
    userAddr.stripeLoc = IN_USER_AREA;
}

StripeTestFixture::~StripeTestFixture(void)
{
}

StripeId
StripeTestFixture::GetVsid(void) const
{
    return vsid;
}

int
StripeTestFixture::GetVolumeId(void) const
{
    return volId;
}

uint32_t
StripeTestFixture::GetWbLsid(int vsid)
{
    return (uint32_t)(vsid);
    // TODO (huijeong.kim) need to change to
    // return (uint32_t)(vsid % testInfo->numWbStripes);
}

StripeId
StripeTestFixture::GetUserLsid(int vsid, TestInfo* testInfo)
{
    return vsid;
}

StripeAddr
StripeTestFixture::GetWbAddr(void) const
{
    return wbAddr;
}

StripeAddr
StripeTestFixture::GetUserAddr(void) const
{
    return userAddr;
}

BlockMapList
StripeTestFixture::GetBlockMapList(void) const
{
    return blks;
}

void
StripeTestFixture::AddBlockMap(BlkAddr rba, VirtualBlks virtualBlks)
{
    blks.push_back(std::make_pair(rba, virtualBlks));
    for (int offset = 0; offset < virtualBlks.numBlks; offset++)
    {
        revMapInfos[virtualBlks.startVsa.offset + offset] = rba + offset;
    }
}

std::map<uint64_t, BlkAddr>
StripeTestFixture::GetRevMap(void)
{
    return revMapInfos;
}
} // namespace pos
