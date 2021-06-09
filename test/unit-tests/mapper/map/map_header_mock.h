#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map_header.h"

namespace pos
{
class MockMpageValidInfo : public MpageValidInfo
{
public:
    using MpageValidInfo::MpageValidInfo;
};

class MockMapHeader : public MapHeader
{
public:
    using MapHeader::MapHeader;
    MOCK_METHOD(BitMap*, GetMpageMap, (), (override));
    MOCK_METHOD(void, SetMapAllocated, (int pageNr), (override));
    MOCK_METHOD(BitMap*, GetTouchedMpages, (), (override));
    MOCK_METHOD(uint32_t, GetMpageSize, (), (override));
    MOCK_METHOD(void, SetMpageSize, (uint32_t mpageSize_), (override));
    MOCK_METHOD(uint32_t, GetEntriesPerMpage, (), (override));
    MOCK_METHOD(void, SetEntriesPerMpage, (uint32_t entriesPerMpage_), (override));
};

}   // namespace pos
