#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/map/map.h"

namespace pos
{
class MockMpage : public Mpage
{
public:
    using Mpage::Mpage;
};

class MockMap : public Map
{
public:
    using Map::Map;
    MOCK_METHOD(uint64_t, GetSize, (), (override));
    MOCK_METHOD(uint64_t, GetNumMpages, (), (override));
    MOCK_METHOD(char*, GetMpage, (uint64_t pageNr), (override));
    MOCK_METHOD(char*, GetMpageWithLock, (uint64_t pageNr), (override));
    MOCK_METHOD(char*, AllocateMpage, (uint64_t pageNr), (override));
    MOCK_METHOD(void, GetMpageLock, (uint64_t pageNr), (override));
    MOCK_METHOD(void, ReleaseMpageLock, (uint64_t pageNr), (override));
};

} // namespace pos
