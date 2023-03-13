#include "vsamap_fake.h"
#include "src/mapper/include/mapper_const.h"

using ::testing::AtLeast;

namespace pos
{
VSAMapFake::VSAMapFake(TestInfo* testInfo)
: testInfo(testInfo)
{
    map = new VirtualBlkAddr*[testInfo->maxNumVolume];
    for (int volumeId = 0; volumeId < testInfo->maxNumVolume; volumeId++)
    {
        map[volumeId] = new VirtualBlkAddr[testInfo->maxVolumeSizeInBlock];
        for (uint64_t blk = 0; blk < testInfo->maxVolumeSizeInBlock; blk++)
        {
            map[volumeId][blk] = UNMAP_VSA;
        }
    }

    ON_CALL(*this, SetVSAsInternal).WillByDefault(::testing::Invoke(this, &VSAMapFake::_SetVSAsInternal));
    ON_CALL(*this, SetVSAsWithSyncOpen).WillByDefault(::testing::Invoke(this, &VSAMapFake::_SetVSAsWithSyncOpen));
    EXPECT_CALL(*this, SetVSAsWithSyncOpen).Times(AtLeast(0));
}

VSAMapFake::~VSAMapFake(void)
{
    for (int idx = 0; idx < testInfo->maxNumVolume; idx++)
    {
        delete [] map[idx];
    }
    delete [] map;
}

VirtualBlkAddr
VSAMapFake::GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    assert(volumeId < testInfo->maxNumVolume);
    assert(rba < testInfo->maxVolumeSizeInBlock);

    caller = OK_READY;
    return map[volumeId][rba];
}

VirtualBlkAddr
VSAMapFake::GetVSAWithSyncOpen(int volId, BlkAddr rba)
{
    assert(volId < testInfo->maxNumVolume);
    assert(rba < testInfo->maxVolumeSizeInBlock);

    return map[volId][rba];
}

MpageList
VSAMapFake::GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks)
{
    MpageList dirty;
    int numEntriesPerPage = testInfo->metaPageSize / 8;

    MpageNum startMpage = startRba / numEntriesPerPage;
    MpageNum endMpage = (startRba + numBlks - 1) / numEntriesPerPage;

    for (uint64_t page = startMpage; page <= endMpage; page++)
    {
        MpageNum pageNum = (int)startRba / numEntriesPerPage;
        dirty.insert(pageNum);
    }
    return dirty;
}

int
VSAMapFake::_SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    assert(volumeId < testInfo->maxNumVolume);
    std::unique_lock<std::mutex> lock(vsaMapLock);
    for (uint32_t blkCount = 0; blkCount < virtualBlks.numBlks; blkCount++)
    {
        assert(startRba + blkCount < testInfo->maxVolumeSizeInBlock);
        map[volumeId][startRba + blkCount].stripeId = virtualBlks.startVsa.stripeId;
        map[volumeId][startRba + blkCount].offset = virtualBlks.startVsa.offset + blkCount;
    }
    return 0;
}

int
VSAMapFake::_SetVSAsWithSyncOpen(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    std::unique_lock<std::mutex> lock(vsaMapLock);
    assert(volId < testInfo->maxNumVolume);
    for (uint32_t blkCount = 0; blkCount < virtualBlks.numBlks; blkCount++)
    {
        assert(startRba + blkCount < testInfo->maxVolumeSizeInBlock);
        map[volId][startRba + blkCount].stripeId = virtualBlks.startVsa.stripeId;
        map[volId][startRba + blkCount].offset = virtualBlks.startVsa.offset + blkCount;
    }
    return 0;
}

} // namespace pos
