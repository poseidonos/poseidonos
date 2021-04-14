#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_unmounter.h"

namespace pos
{
class MockVolumeUnmounter : public VolumeUnmounter
{
public:
    using VolumeUnmounter::VolumeUnmounter;
};

} // namespace pos
