#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_detacher.h"

namespace pos
{
class MockVolumeDetacher : public VolumeDetacher
{
public:
    using VolumeDetacher::VolumeDetacher;
};

} // namespace pos
