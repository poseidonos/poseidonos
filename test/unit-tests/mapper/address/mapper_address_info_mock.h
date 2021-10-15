#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/mapper/address/mapper_address_info.h"

namespace pos
{
class MockMapperAddressInfo : public MapperAddressInfo
{
public:
    using MapperAddressInfo::MapperAddressInfo;
    MOCK_METHOD(void, SetupAddressInfo, (int mpageSize_), (override));
    MOCK_METHOD(std::string, GetArrayName, (), (override));
    MOCK_METHOD(int, GetArrayId, (), (override));
    MOCK_METHOD(bool, IsUT, (), (override));
    MOCK_METHOD(uint32_t, GetBlksPerStripe, (), (override));
    MOCK_METHOD(uint32_t, GetNumWbStripes, (), (override));
    MOCK_METHOD(uint32_t, GetMaxVSID, (), (override));
    MOCK_METHOD(uint64_t, GetMpageSize, (), (override));
};

} // namespace pos
