#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/reversemap/reverse_map.h"

namespace pos
{
class MockRevMapEntry : public RevMapEntry
{
public:
    using RevMapEntry::RevMapEntry;
};

class MockRevMapSector : public RevMapSector
{
public:
    using RevMapSector::RevMapSector;
};

class MockRevMap : public RevMap
{
public:
    using RevMap::RevMap;
};

class MockReverseMapPage : public ReverseMapPage
{
public:
    using ReverseMapPage::ReverseMapPage;
};

class MockReverseMapPack : public ReverseMapPack
{
public:
    using ReverseMapPack::ReverseMapPack;
    MOCK_METHOD(int, SetReverseMapEntry, (uint64_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (uint64_t offset), (override));
    MOCK_METHOD(std::vector<ReverseMapPage>, GetReverseMapPages, (), (override));
    MOCK_METHOD(int, HeaderLoaded, (), (override));
    MOCK_METHOD(char*, GetRevMapPtrForWBT, (), (override));
    MOCK_METHOD(StripeId, GetVsid, (), (override));
};

} // namespace pos
