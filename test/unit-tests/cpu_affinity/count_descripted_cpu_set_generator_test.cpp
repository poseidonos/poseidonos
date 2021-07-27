#include "src/cpu_affinity/count_descripted_cpu_set_generator.h"

#include <gtest/gtest.h>

namespace pos
{
const CoreDescriptionArray TEST_CORE_DESCRIPTIONS =
    {
        CoreDescription{CoreType::REACTOR, {1, 0}, "0"},
        CoreDescription{CoreType::UDD_IO_WORKER, {1, 0}, "1"},
        CoreDescription{CoreType::EVENT_SCHEDULER, {1, 0}, "2"},
        CoreDescription{CoreType::EVENT_WORKER, {3, 0}, "3-5"},
        CoreDescription{CoreType::GENERAL_USAGE, {1, 0}, "6"},
        CoreDescription{CoreType::QOS, {1, 0}, "7"},
        CoreDescription{CoreType::META_SCHEDULER, {1, 0}, "8"},
        CoreDescription{CoreType::META_IO, {2, 0}, "9-10"},
        CoreDescription{CoreType::AIR, {1, 0}, "11"},
};

TEST(CountDescriptedCpuSetGenerator, CountDescriptedCpuSetGenerator_Stack)
{
    // Given

    // When : Create CountDescriptedCpuSetGenerator on stack
    CountDescriptedCpuSetGenerator countDescriptedCpuSetGenerator(TEST_CORE_DESCRIPTIONS);

    // Then : Do nothing
}

TEST(CountDescriptedCpuSetGenerator, CountDescriptedCpuSetGenerator_Heap)
{
    // Given

    // When : Create CountDescriptedCpuSetGenerator on heap
    CountDescriptedCpuSetGenerator* countDescriptedCpuSetGenerator = new CountDescriptedCpuSetGenerator(TEST_CORE_DESCRIPTIONS);

    // Then : Release memory
    delete countDescriptedCpuSetGenerator;
}

} // namespace pos
