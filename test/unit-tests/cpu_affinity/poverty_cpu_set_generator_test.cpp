#include "src/cpu_affinity/poverty_cpu_set_generator.h"

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

TEST(PovertyCpuSetGenerator, PovertyCpuSetGenerator_Stack)
{
    // Given

    // When : Create PovertyCpuSetGenerator on stack
    PovertyCpuSetGenerator povertyCpuSetGenerator(TEST_CORE_DESCRIPTIONS);

    // Then : Do nothing
}

TEST(PovertyCpuSetGenerator, PovertyCpuSetGenerator_Heap)
{
    // Given

    // When : Create PovertyCpuSetGenerator on heap
    PovertyCpuSetGenerator* povertyCpuSetGenerator = new PovertyCpuSetGenerator(TEST_CORE_DESCRIPTIONS);

    // Then : Release memory
    delete povertyCpuSetGenerator;
}

} // namespace pos
