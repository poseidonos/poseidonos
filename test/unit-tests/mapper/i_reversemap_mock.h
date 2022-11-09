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
    MOCK_METHOD(int, Load, (ReverseMapPack * rev, EventSmartPtr cb), (override));
    MOCK_METHOD(int, Flush, (ReverseMapPack * rev, EventSmartPtr cb), (override));
    MOCK_METHOD(ReverseMapPack*, AllocReverseMapPack, (StripeId vsid, StripeId wblsid), (override));
    MOCK_METHOD(int, ReconstructReverseMap, (uint32_t volumeId, uint64_t totalRba, uint32_t wblsid, uint32_t vsid, uint64_t blockCount, (std::map<uint64_t, BlkAddr> revMapInfos), ReverseMapPack* revMapPack), (override));
};

} // namespace pos
