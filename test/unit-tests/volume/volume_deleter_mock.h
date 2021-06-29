#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_deleter.h"

namespace pos
{
class MockVolumeDeleter : public VolumeDeleter
{
public:
    using VolumeDeleter::VolumeDeleter;
};

} // namespace pos
