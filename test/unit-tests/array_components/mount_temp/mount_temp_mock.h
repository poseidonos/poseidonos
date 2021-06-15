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
    MOCK_METHOD(int, Mount1, (), (override));
    MOCK_METHOD(int, Unmount2, (), (override));
};

} // namespace pos
