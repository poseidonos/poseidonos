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
    MOCK_METHOD(void, Init, (uint64_t mpsize, uint64_t nmpPerStripe, MetaFileIntf* file, std::string arrName), (override));
    MOCK_METHOD(void, Init, (IVolumeManager * volumeManager, StripeId wblsid, VSAMapManager* ivsaMap, IStripeMap* istripeMap), (override));
    MOCK_METHOD(int, LinkVsid, (StripeId vsid), (override));
    MOCK_METHOD(int, UnLinkVsid, (), (override));
    MOCK_METHOD(int, Load, (EventSmartPtr callback), (override));
    MOCK_METHOD(int, Flush, (Stripe * stripe, EventSmartPtr callback), (override));
    MOCK_METHOD(int, SetReverseMapEntry, (uint32_t offset, BlkAddr rba, uint32_t volumeId), (override));
    MOCK_METHOD(int, ReconstructMap, (uint32_t volumeId, StripeId vsid, StripeId lsid, uint64_t blockCount, (std::map<uint64_t, BlkAddr> revMapInfos)), (override));
    MOCK_METHOD((std::tuple<BlkAddr, uint32_t>), GetReverseMapEntry, (uint32_t offset), (override));
    MOCK_METHOD(int, IsAsyncIoDone, (), (override));
    MOCK_METHOD(int, GetIoError, (), (override));
    MOCK_METHOD(int, WbtFileSyncIo, (MetaFileIntf * fileLinux, MetaFsIoOpcode IoDirection), (override));
};

} // namespace pos
