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
};

} // namespace pos
