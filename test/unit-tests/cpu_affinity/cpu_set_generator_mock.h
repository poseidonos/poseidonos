#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cpu_affinity/cpu_set_generator.h"

namespace pos
{
class MockCoreDescription : public CoreDescription
{
public:
    using CoreDescription::CoreDescription;
};

class MockCpuSetGenerator : public CpuSetGenerator
{
public:
    using CpuSetGenerator::CpuSetGenerator;
};

} // namespace pos
