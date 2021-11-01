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
    MOCK_METHOD(int, Load, (ReverseMapPack * rev, StripeId wblsid, StripeId vsid, EventSmartPtr cb), (override));
    MOCK_METHOD(int, UpdateReverseMapEntry, (ReverseMapPack * rev, StripeId wblsid, uint64_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (ReverseMapPack * rev, StripeId wblsid, uint64_t offset), (override));
    MOCK_METHOD(ReverseMapPack*, Assign, (StripeId wblsid, StripeId vsid), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (uint32_t vsid), (override));
    MOCK_METHOD(int, ReconstructReverseMap, (uint32_t volumeId, uint64_t totalRba, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD(int, Flush, (ReverseMapPack * rev, StripeId wblsid, Stripe* stripe, StripeId vsid, EventSmartPtr cb), (override));
};

} // namespace pos
