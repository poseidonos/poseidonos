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

class MockRevMapPageAsyncIoCtx : public RevMapPageAsyncIoCtx
{
public:
    using RevMapPageAsyncIoCtx::RevMapPageAsyncIoCtx;
};

class MockReverseMapPack : public ReverseMapPack
{
public:
    using ReverseMapPack::ReverseMapPack;
};

} // namespace pos
