#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array_models/dto/device_set.h"

namespace pos
{
template<typename T>
class MockDeviceSet : public DeviceSet<T>
{
public:
    using DeviceSet::DeviceSet;
};

} // namespace pos
