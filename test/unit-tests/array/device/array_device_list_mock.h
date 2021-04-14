#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/device/array_device_list.h"

namespace pos
{
class MockArrayDeviceList : public ArrayDeviceList
{
public:
    using ArrayDeviceList::ArrayDeviceList;
};

} // namespace pos
