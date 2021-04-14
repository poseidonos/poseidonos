#include "test/integration-tests/journal/utils/test_info.h"
namespace pos
{
const std::string ARRAY_NAME = "POSArray";

TestInfo::TestInfo(void)
{
    numWbStripes = 64;
    numStripesPerSegment = 64;
    numUserSegments = 64;
    numUserStripes = numStripesPerSegment * numUserSegments;

    numBlksPerChunk = 32;
    numChunksPerStripe = 4;
    numBlksPerStripe = numBlksPerChunk * numChunksPerStripe;
    numUserBlocks = numUserStripes * numBlksPerStripe;

    numVolumeMap = 256;
    numMap = numVolumeMap + 1;
    defaultTestVol = 1;
    maxNumVolume = 5;
    numTest = 1000;

    maxVolumeSizeInBlock = numUserBlocks / maxNumVolume;
    defaultTestVolSizeInBlock = maxVolumeSizeInBlock; // 1GB
    metaPageSize = 4032;
    metaPartitionSize = 16 * 1024 * 1024;
}

void
TestInfo::SetNumStripesPerSegment(int value)
{
    numStripesPerSegment = value;
    numUserStripes = numStripesPerSegment * numUserSegments;
}
} // namespace pos
