#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/metafs/mim/mdpage_control_info.h"

namespace pos
{
class MockMDPageControlInfo : public MDPageControlInfo
{
public:
    using MDPageControlInfo::MDPageControlInfo;
};

} // namespace pos
