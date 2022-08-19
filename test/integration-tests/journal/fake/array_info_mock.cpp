#include "array_info_mock.h"

#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
ArrayInfoMock::ArrayInfoMock(TestInfo* _testInfo)
{
    testInfo = _testInfo;
    userSizeInfo = new PartitionLogicalSize();
    wbSizeInfo = new PartitionLogicalSize();
}

ArrayInfoMock::~ArrayInfoMock(void)
{
    delete wbSizeInfo;
    delete userSizeInfo;
}

const PartitionLogicalSize*
ArrayInfoMock::GetSizeInfo(PartitionType type)
{
    if (PartitionType::USER_DATA == type)
    {
        userSizeInfo->minWriteBlkCnt = 0;
        userSizeInfo->blksPerChunk = testInfo->numBlksPerChunk;
        userSizeInfo->blksPerStripe = testInfo->numBlksPerStripe;
        userSizeInfo->chunksPerStripe = testInfo->numChunksPerStripe;
        userSizeInfo->stripesPerSegment = testInfo->numStripesPerSegment;
        userSizeInfo->totalSegments = testInfo->numUserSegments;
        userSizeInfo->totalStripes = testInfo->numUserStripes;
        return userSizeInfo;
    }
    else if (PartitionType::WRITE_BUFFER == type)
    {
        userSizeInfo->minWriteBlkCnt = 0;
        userSizeInfo->blksPerChunk = testInfo->numBlksPerChunk;
        userSizeInfo->blksPerStripe = testInfo->numBlksPerStripe;
        userSizeInfo->chunksPerStripe = testInfo->numChunksPerStripe;
        userSizeInfo->stripesPerSegment = testInfo->numStripesPerSegment;
        userSizeInfo->totalSegments = 1;
        userSizeInfo->totalStripes = testInfo->numWbStripes;
        return wbSizeInfo;
    }

    return nullptr;
}

DeviceSet<string>
ArrayInfoMock::GetDevNames(void)
{
    DeviceSet<string> ret;
    return ret;
}

string
ArrayInfoMock::GetName(void)
{
    return "";
}

unsigned int
ArrayInfoMock::GetIndex(void)
{
    return 0;
}

string
ArrayInfoMock::GetMetaRaidType(void)
{
    return "";
}

string
ArrayInfoMock::GetDataRaidType(void)
{
    return "";
}

id_t 
ArrayInfoMock::GetUniqueId(void)
{
    return 0;
}

ArrayStateType
ArrayInfoMock::GetState(void)
{
    return ArrayStateEnum::NORMAL;
}

StateContext*
ArrayInfoMock::GetStateCtx(void)
{
    return nullptr;
}

uint32_t
ArrayInfoMock::GetRebuildingProgress(void)
{
    return 0;
}

string
ArrayInfoMock::GetCreateDatetime(void)
{
    return "";
}

string
ArrayInfoMock::GetUpdateDatetime(void)
{
    return "";
}

bool
ArrayInfoMock::IsWriteThroughEnabled(void)
{
    return false;
}
} // namespace pos
