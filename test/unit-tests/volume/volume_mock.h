#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume.h"

namespace pos
{
class MockVolume : public Volume
{
public:
    using Volume::Volume;
};

} // namespace pos
