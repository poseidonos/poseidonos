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
    MOCK_METHOD(void, Init, (MetaFileIntf * file, StripeId wbLsid_, StripeId vsid_, uint32_t mpageSize_, uint32_t numMpagesPerStripe_, TelemetryPublisher* tp), (override));
    MOCK_METHOD(void, Assign, (StripeId vsid), (override));
    MOCK_METHOD(int, Load, (uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid), (override));
    MOCK_METHOD(int, Flush, (Stripe * stripe, uint64_t fileOffset, EventSmartPtr cb, uint32_t vsid), (override));
    MOCK_METHOD(int, SetReverseMapEntry, (uint64_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (uint64_t offset), (override));
    MOCK_METHOD(char*, GetRevMapPtrForWBT, (), (override));
    MOCK_METHOD(void, WaitPendingIoDone, (), (override));
};

} // namespace pos
