#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_resizer.h"

namespace pos
{
class MockVolumeResizer : public VolumeResizer
{
public:
    using VolumeResizer::VolumeResizer;
};

} // namespace pos
