#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/array/build/device_builder.h"

namespace pos
{
class MockDeviceBuilder : public DeviceBuilder
{
public:
    using DeviceBuilder::DeviceBuilder;
};

} // namespace pos
