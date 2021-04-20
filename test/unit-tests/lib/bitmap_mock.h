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
    MOCK_METHOD(bool, SetBit, (uint64_t bitOffset), (override));
    MOCK_METHOD(bool, ClearBit, (uint64_t bitOffset), (override));
    MOCK_METHOD(bool, ClearBits, (uint64_t begin, uint64_t end), (override));
    MOCK_METHOD(void, ResetBitmap, (), (override));
    MOCK_METHOD(bool, IsSetBit, (uint64_t bit), (override));
    MOCK_METHOD(void, FlipBit, (uint64_t bit), (override));
    MOCK_METHOD(uint64_t, FindFirstSet, (uint64_t begin), (override));
    MOCK_METHOD(uint64_t, FindFirstZero, (), (override));
    MOCK_METHOD(uint64_t, FindFirstZero, (uint64_t begin), (override));
    MOCK_METHOD(uint64_t, FindFirstZero, (uint64_t begin, uint64_t end), (override));
    MOCK_METHOD(uint64_t, FindNextZero, (), (override));
    MOCK_METHOD(bool, IsValidBit, (uint64_t bitOffset), (override));
    MOCK_METHOD(bool, Set, (BitMap & inputBitMap), (override));
};

class MockBitMapMutex : public BitMapMutex
{
public:
    using BitMapMutex::BitMapMutex;
};

} // namespace pos
