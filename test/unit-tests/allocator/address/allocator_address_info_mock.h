#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/allocator/address/allocator_address_info.h"

namespace pos
{
class MockAllocatorAddressInfo : public AllocatorAddressInfo
{
public:
    using AllocatorAddressInfo::AllocatorAddressInfo;
    MOCK_METHOD(void, Init, (std::string arrayName, IArrayInfo* iArrayInfo), (override));
    MOCK_METHOD(uint32_t, GetnumUserAreaSegments, (), (override));
};

} // namespace pos
