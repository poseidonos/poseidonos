#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/uram/uram_device_context.h"

namespace pos
{
class MockUramDeviceContext : public UramDeviceContext
{
public:
    using UramDeviceContext::UramDeviceContext;
};

} // namespace pos
