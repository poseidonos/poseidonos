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
    MOCK_METHOD(char*, GetMpage, (int pageNr), (override));
    MOCK_METHOD(char*, GetMpageWithLock, (int pageNr), (override));
    MOCK_METHOD(char*, AllocateMpage, (int pageNr), (override));
    MOCK_METHOD(void, GetMpageLock, (int pageNr), (override));
    MOCK_METHOD(void, ReleaseMpageLock, (int pageNr), (override));
};

} // namespace pos
