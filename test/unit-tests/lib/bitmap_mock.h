#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/lib/bitmap.h"

namespace pos
{
class MockBitMap : public BitMap
{
public:
    using BitMap::BitMap;
};

class MockBitMapMutex : public BitMapMutex
{
public:
    using BitMapMutex::BitMapMutex;
};

} // namespace pos
