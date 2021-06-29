#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_mounter.h"

namespace pos
{
class MockVolumeMounter : public VolumeMounter
{
public:
    using VolumeMounter::VolumeMounter;
};

} // namespace pos
