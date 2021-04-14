#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/device/unvme/unvme_device_context.h"

namespace pos
{
class MockUnvmeDeviceContext : public UnvmeDeviceContext
{
public:
    using UnvmeDeviceContext::UnvmeDeviceContext;
};

} // namespace pos
