#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/i_vsamap.h"

namespace pos
{
class MockIVSAMap : public IVSAMap
{
public:
    using IVSAMap::IVSAMap;
    MOCK_METHOD(int, GetVSAs, (int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray), (override));
    MOCK_METHOD(int, SetVSAs, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAInternal, (int volumeId, BlkAddr rba, int& caller), (override));
    MOCK_METHOD(int, SetVSAsInternal, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAWithSyncOpen, (int volumeId, BlkAddr rba), (override));
    MOCK_METHOD(int, SetVSAsWithSyncOpen, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetRandomVSA, (BlkAddr rba), (override));
    MOCK_METHOD(MpageList, GetDirtyVsaMapPages, (int volumeId, BlkAddr startRba, uint64_t numBlks), (override));
    MOCK_METHOD(int64_t, GetNumUsedBlks, (int volId), (override));
};

} // namespace pos
