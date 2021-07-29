#pragma once

#include <map>
#include <utility>
#include <vector>

#include "src/include/address_type.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
static const StripeAddr unmapAddr = {
    .stripeLoc = IN_WRITE_BUFFER_AREA,
    .stripeId = UNMAP_STRIPE};

using BlockMapList = std::vector<std::pair<BlkAddr, VirtualBlks>>;

class StripeTestFixture
{
public:
    StripeTestFixture(void);
    StripeTestFixture(int _vsid, int _volId);
    virtual ~StripeTestFixture(void);

    StripeId GetVsid(void) const;
    int GetVolumeId(void) const;

    // TODO (huijeong.kim) Refactoring required, remove static functions
    // Two static functions below are for calculating address
    static uint32_t GetWbLsid(int vsid);
    static StripeId GetUserLsid(int vsid, TestInfo* testInfo);

    StripeAddr GetWbAddr(void) const;
    StripeAddr GetUserAddr(void) const;

    BlockMapList GetBlockMapList(void) const;
    void AddBlockMap(BlkAddr rba, VirtualBlks virtualBlks);

    std::map<uint64_t, BlkAddr> GetRevMap(void);

private:
    StripeId vsid;
    int volId;

    StripeAddr wbAddr;
    StripeAddr userAddr;

    BlockMapList blks;
    std::map<uint64_t, BlkAddr> revMapInfos;
};
} // namespace pos
