#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/mapper.h"

namespace pos
{
class MockMapper : public Mapper
{
public:
    using Mapper::Mapper;
    MOCK_METHOD(int, Init, (), (override));
    MOCK_METHOD(void, Dispose, (), (override));
    MOCK_METHOD(IVSAMap*, GetIVSAMap, (), (override));
    MOCK_METHOD(IStripeMap*, GetIStripeMap, (), (override));
    MOCK_METHOD(IMapFlush*, GetIMapFlush, (), (override));
    MOCK_METHOD(int, FlushDirtyMpages, (int mapId, EventSmartPtr callback, MpageList dirtyPages), (override));
    MOCK_METHOD(int, FlushAllMaps, (), (override));
    MOCK_METHOD(void, WaitForFlushAllMapsDone, (), (override));
    MOCK_METHOD(int, StoreAllMaps, (), (override));
    MOCK_METHOD(int, GetMapLayout, (std::string fname), (override));
    MOCK_METHOD(int, ReadVsaMap, (int volId, std::string fname), (override));
    MOCK_METHOD(int, ReadVsaMapEntry, (int volId, BlkAddr rba, std::string fname), (override));
    MOCK_METHOD(int, WriteVsaMap, (int volId, std::string fname), (override));
    MOCK_METHOD(int, WriteVsaMapEntry, (int volId, BlkAddr rba, VirtualBlkAddr vsa), (override));
    MOCK_METHOD(int, ReadStripeMap, (std::string fname), (override));
    MOCK_METHOD(int, ReadStripeMapEntry, (StripeId vsid, std::string fname), (override));
    MOCK_METHOD(int, WriteStripeMap, (std::string fname), (override));
    MOCK_METHOD(int, WriteStripeMapEntry, (StripeId vsid, StripeLoc loc, StripeId lsid), (override));
    MOCK_METHOD(int, ReadReverseMap, (StripeId vsid, std::string fname), (override));
    MOCK_METHOD(int, ReadWholeReverseMap, (std::string fname), (override));
    MOCK_METHOD(int, ReadReverseMapEntry, (StripeId vsid, BlkOffset offset, std::string fname), (override));
    MOCK_METHOD(int, WriteReverseMap, (StripeId vsid, std::string fname), (override));
    MOCK_METHOD(int, WriteWholeReverseMap, (std::string fname), (override));
    MOCK_METHOD(int, WriteReverseMapEntry, (StripeId vsid, BlkOffset offset, BlkAddr rba, uint32_t volumeId), (override));
};

} // namespace pos
