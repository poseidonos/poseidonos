#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/parameters_volume.h"

namespace pos
{
class MockVolumeParameter : public VolumeParameter
{
public:
    using VolumeParameter::VolumeParameter;
};

} // namespace pos
