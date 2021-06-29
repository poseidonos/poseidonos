#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_qos_updater.h"

namespace pos
{
class MockVolumeQosUpdater : public VolumeQosUpdater
{
public:
    using VolumeQosUpdater::VolumeQosUpdater;
};

} // namespace pos
