#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/poverty_cpu_set_generator.h"

namespace pos
{
class MockPovertyCpuSetGenerator : public PovertyCpuSetGenerator
{
public:
    using PovertyCpuSetGenerator::PovertyCpuSetGenerator;
};

} // namespace pos
