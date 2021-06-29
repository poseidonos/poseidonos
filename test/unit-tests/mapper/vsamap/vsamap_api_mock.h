#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/vsamap/vsamap_api.h"

namespace pos
{
class MockVSAMapAPI : public VSAMapAPI
{
public:
    using VSAMapAPI::VSAMapAPI;
    MOCK_METHOD(int, GetVSAs, (int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray), (override));
    MOCK_METHOD(int, SetVSAs, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(VirtualBlkAddr, GetVSAInternal, (int volumeId, BlkAddr rba, int& caller), (override));
    MOCK_METHOD(int, SetVSAsInternal, (int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks), (override));
    MOCK_METHOD(MpageList, GetDirtyVsaMapPages, (int volumeId, BlkAddr startRba, uint64_t numBlks), (override));
    MOCK_METHOD(int64_t, GetNumUsedBlocks, (int volId), (override));
};

} // namespace pos
