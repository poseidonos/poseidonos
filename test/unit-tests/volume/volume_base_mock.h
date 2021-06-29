#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_base.h"

namespace pos
{
class MockVolumeBase : public VolumeBase
{
public:
    using VolumeBase::VolumeBase;
};

} // namespace pos
