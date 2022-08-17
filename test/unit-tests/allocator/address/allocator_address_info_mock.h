#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"

using ::testing::Return;

namespace pos
{
class MockAllocatorAddressInfo : public AllocatorAddressInfo
{
public:
    using AllocatorAddressInfo::AllocatorAddressInfo;
    MOCK_METHOD(void, Init, (IArrayInfo * iArrayInfo), (override));
    MOCK_METHOD(uint32_t, GetblksPerStripe, (), (override));
    MOCK_METHOD(uint32_t, GetchunksPerStripe, (), (override));
    MOCK_METHOD(uint32_t, GetnumWbStripes, (), (override));
    MOCK_METHOD(uint32_t, GetnumUserAreaStripes, (), (override));
    MOCK_METHOD(uint32_t, GetblksPerSegment, (), (override));
    MOCK_METHOD(uint32_t, GetstripesPerSegment, (), (override));
    MOCK_METHOD(uint32_t, GetnumUserAreaSegments, (), (override));
    MOCK_METHOD(bool, IsUT, (), (override));
};
} // namespace pos
