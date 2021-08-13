#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array_components/components_info.h"

namespace pos
{
class MockComponentsInfo : public ComponentsInfo
{
public:
    using ComponentsInfo::ComponentsInfo;
};

} // namespace pos
