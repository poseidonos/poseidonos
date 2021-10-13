#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters_all_volumes.h"

namespace pos
{
class MockAllVolumeParameter : public AllVolumeParameter
{
public:
    using AllVolumeParameter::AllVolumeParameter;
};

} // namespace pos
