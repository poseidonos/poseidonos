#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_creator.h"

namespace pos
{
class MockVolumeCreator : public VolumeCreator
{
public:
    using VolumeCreator::VolumeCreator;
};

} // namespace pos
