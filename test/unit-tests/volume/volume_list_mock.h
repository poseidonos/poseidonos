#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/volume/volume_list.h"

namespace pos
{
class MockVolumeList : public VolumeList
{
public:
    using VolumeList::VolumeList;
};

} // namespace pos
