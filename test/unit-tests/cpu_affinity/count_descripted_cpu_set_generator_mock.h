#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/count_descripted_cpu_set_generator.h"

namespace pos
{
class MockCountDescriptedCpuSetGenerator : public CountDescriptedCpuSetGenerator
{
public:
    using CountDescriptedCpuSetGenerator::CountDescriptedCpuSetGenerator;
};

} // namespace pos
