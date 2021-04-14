#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_renamer.h"

namespace pos
{
class MockVolumeRenamer : public VolumeRenamer
{
public:
    using VolumeRenamer::VolumeRenamer;
};

} // namespace pos
