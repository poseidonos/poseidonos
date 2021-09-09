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
    MOCK_METHOD(BitMap*, GetMpageMap, (), (override));
    MOCK_METHOD(void, SetMapAllocated, (int pageNr), (override));
    MOCK_METHOD(BitMap*, GetTouchedMpages, (), (override));
};

} // namespace pos
