#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/base/device_context.h"

namespace pos
{
class MockDeviceContext : public DeviceContext
{
public:
    using DeviceContext::DeviceContext;
};

} // namespace pos
