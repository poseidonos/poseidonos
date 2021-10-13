#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/qos/resource_cpu.h"

namespace pos
{
class MockResourceCpu : public ResourceCpu
{
public:
    using ResourceCpu::ResourceCpu;
};

} // namespace pos
