#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/sys_info/space_info.h"

namespace pos
{
class MockSpaceInfo : public SpaceInfo
{
public:
    using SpaceInfo::SpaceInfo;
};

} // namespace pos
