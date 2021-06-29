#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_loader.h"

namespace pos
{
class MockVolumeLoader : public VolumeLoader
{
public:
    using VolumeLoader::VolumeLoader;
};

} // namespace pos
