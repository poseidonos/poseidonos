#pragma once
#include <cstdint>
#include <cstdlib>
#include <string>

#include "gtest/gtest.h"

namespace pos
{

static std::string GetLogFileName(void)
{
    std::string test_suite = ::testing::UnitTest::GetInstance()->current_test_suite()->name();
    std::string test = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string filename = test_suite + "_" + test + ".bin";

    return filename;
}

static std::string GetLogDirName()
{
    std::string test_suite = ::testing::UnitTest::GetInstance()->current_test_suite()->name();
    std::string test = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    std::string filename = test_suite + "_" + test;

    return filename;
}
// Test setup variables
// TODO (cheolho.kang) Change it to global variable or singleton instance
class TestInfo
{
public:
    TestInfo(void);
    void SetNumStripesPerSegment(int value);

    int numWbStripes;
    int numStripesPerSegment;
    int numUserSegments;
    uint32_t numUserStripes;

    int numBlksPerChunk;
    int numChunksPerStripe;
    uint64_t numBlksPerStripe;
    uint64_t numUserBlocks;

    int numVolumeMap;
    int numMap;
    int defaultTestVol;
    int maxNumVolume;
    uint32_t numTest;

    uint64_t maxVolumeSizeInBlock;
    uint64_t defaultTestVolSizeInBlock;

    uint64_t metaPageSize;
    uint64_t metaPartitionSize;

    static const std::string ARRAY_NAME;
};
} // namespace pos
