#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_components/mount_temp/mount_temp.h"

namespace pos
{
class MockMountTemp : public MountTemp
{
public:
    using MountTemp::MountTemp;
};

} // namespace pos
