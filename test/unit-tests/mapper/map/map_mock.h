#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map.h"

namespace pos
{
class MockMpageValidInfo : public MpageValidInfo
{
public:
    using MpageValidInfo::MpageValidInfo;
};

class MockMpage : public Mpage
{
public:
    using Mpage::Mpage;
};

class MockMapHeader : public MapHeader
{
public:
    using MapHeader::MapHeader;
};

class MockMap : public Map
{
public:
    using Map::Map;
};

} // namespace pos
