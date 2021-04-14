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
};

} // namespace pos
