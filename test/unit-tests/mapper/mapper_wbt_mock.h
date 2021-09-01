#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/mapper_wbt.h"

namespace pos
{
class MockMapperWbt : public MapperWbt
{
public:
    using MapperWbt::MapperWbt;
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
