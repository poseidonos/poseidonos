#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/string_descripted_cpu_set_generator.h"

namespace pos
{
class MockStringDescriptedCpuSetGenerator : public StringDescriptedCpuSetGenerator
{
public:
    using StringDescriptedCpuSetGenerator::StringDescriptedCpuSetGenerator;
};

} // namespace pos
