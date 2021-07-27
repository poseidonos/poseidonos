#include "src/cpu_affinity/string_descripted_cpu_set_generator.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(StringDescriptedCpuSetGenerator, StringDescriptedCpuSetGenerator_Stack_NonMask)
{
    // Given
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
    // When : Create StringDescriptedCpuSetGenerator on stack
    StringDescriptedCpuSetGenerator stringDescriptedCpuSetGenerator(TEST_CORE_DESCRIPTIONS, false);

    // Then : Do nothing
}

TEST(StringDescriptedCpuSetGenerator, StringDescriptedCpuSetGenerator_Heap_Mask)
{
    // Given
    const CoreDescriptionArray TEST_CORE_DESCRIPTIONS =
        {
            CoreDescription{CoreType::REACTOR, {1, 0}, "0x0"},
            CoreDescription{CoreType::UDD_IO_WORKER, {1, 0}, "0x111"},
            CoreDescription{CoreType::EVENT_SCHEDULER, {1, 0}, "2"},
            CoreDescription{CoreType::EVENT_WORKER, {3, 0}, "3-5"},
            CoreDescription{CoreType::GENERAL_USAGE, {1, 0}, "6"},
            CoreDescription{CoreType::QOS, {1, 0}, "7"},
            CoreDescription{CoreType::META_SCHEDULER, {1, 0}, "8"},
            CoreDescription{CoreType::META_IO, {2, 0}, "9-10"},
            CoreDescription{CoreType::AIR, {1, 0}, "11"},
        };
    // When : Create StringDescriptedCpuSetGenerator on stack
    StringDescriptedCpuSetGenerator* stringDescriptedCpuSetGenerator = new StringDescriptedCpuSetGenerator(TEST_CORE_DESCRIPTIONS, false);

    // Then : Release memory
}

} // namespace pos
