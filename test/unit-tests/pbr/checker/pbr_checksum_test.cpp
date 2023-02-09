#include <gtest/gtest.h>
#include "src/pbr/checker/pbr_checksum.h"

TEST(PbrChecksum, PbrChecksum_testWhetherTheSameResultComesOutInTheSameInput)
{
    // Given
    char input[1024];
    uint32_t checksumOffset = 8;
    uint32_t checksumLength = 4;

    for (int i = 0 ; i < 1024; i++)
    {
        input[i] = rand();
    }
    // When
    uint32_t result1 = pbr::MakePbrChecksum(input, 1024, checksumOffset, checksumLength);
    uint32_t result2 = pbr::MakePbrChecksum(input, 1024, checksumOffset, checksumLength);

    // Then
    ASSERT_EQ(result1, result2);
}

TEST(PbrChecksum, PbrChecksum_testIfTheSameValueIsObtainedEvenIfTheValueOfTheChecksumAreaIsDifferent)
{
    // Given
    char input1[1024];
    char input2[1024];
    uint32_t checksumOffset = 8;
    uint32_t checksumLength = 4;

    for (int i = 0 ; i < 1024; i++)
    {
        input1[i] = rand();
        input2[i] = input1[i];
        if (i >= checksumOffset && i < checksumOffset + checksumLength)
        {
            // The value of the checksum area is ignored, so we expect the same result.
            input2[i] = input1[i] + 1;
        }
    }
    // When
    uint32_t result1 = pbr::MakePbrChecksum(input1, 1024, checksumOffset, checksumLength);
    uint32_t result2 = pbr::MakePbrChecksum(input2, 1024, checksumOffset, checksumLength);

    // Then
    ASSERT_EQ(result1, result2);
}