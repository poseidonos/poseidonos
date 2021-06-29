#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_interface.h"

namespace pos
{
class MockVolumeInterface : public VolumeInterface
{
public:
    using VolumeInterface::VolumeInterface;
};

} // namespace pos
