#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_header.h"

namespace pos
{
class MockMpageInfo : public MpageInfo
{
public:
    using MpageInfo::MpageInfo;
};

class MockMapHeader : public MapHeader
{
public:
    using MapHeader::MapHeader;
    MOCK_METHOD(void, Init, (uint64_t numMpages, uint64_t mpageSize), (override));
    MOCK_METHOD(int, CopyToBuffer, (char* buffer), (override));
    MOCK_METHOD(BitMap*, GetBitmapFromTempBuffer, (char* buffer), (override));
    MOCK_METHOD(uint64_t, GetSize, (), (override));
    MOCK_METHOD(void, ApplyHeader, (char* srcBuf), (override));
    MOCK_METHOD(uint64_t, GetNumValidMpages, (), (override));
    MOCK_METHOD(BitMap*, GetMpageMap, (), (override));
    MOCK_METHOD(void, SetMapAllocated, (int pageNr), (override));
    MOCK_METHOD(BitMap*, GetTouchedMpages, (), (override));
    MOCK_METHOD(void, UpdateNumUsedBlks, (VirtualBlkAddr vsa), (override));
    MOCK_METHOD(uint64_t, GetNumUsedBlks, (), (override));
    MOCK_METHOD(int, GetMapId, (), (override));
    MOCK_METHOD(uint32_t, GetNumTouchedMpagesSet, (), (override));
    MOCK_METHOD(uint32_t, GetNumTotalTouchedMpages, (), (override));
    MOCK_METHOD(void, SetTouchedMpageBit, (uint64_t pageNr), (override));
};

} // namespace pos
