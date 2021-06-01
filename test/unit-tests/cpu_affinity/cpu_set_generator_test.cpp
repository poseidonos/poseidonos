#include "src/cpu_affinity/cpu_set_generator.h"

#include <gtest/gtest.h>

namespace pos
{
} // namespace pos

namespace pos
{
TEST(CpuSetGenerator, CpuSetGenerator_Stack)
{
    // Given

    // When : Create CpuSetGenerator on stack
    CpuSetGenerator cpuSetGenerator;

    // Then : Do nothing
}

TEST(CpuSetGenerator, CpuSetGenerator_Heap)
{
    // Given

    // When : Create CpuSetGenerator on heap
    CpuSetGenerator* cpuSetGenerator = new CpuSetGenerator();

    // Then : Release memory
    delete cpuSetGenerator;
}

TEST(CpuSetGenerator, GetCpuSetArray_ReturnCpuSetArray)
{
    // Given

    // When : Create CpuSetGenerator on stack
    CpuSetGenerator cpuSetGenerator;
    cpuSetGenerator.GetCpuSetArray();

    // Then : Do nothing
}

} // namespace pos
