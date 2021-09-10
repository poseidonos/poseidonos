#include "vsamap_mock.h"
#include "src/mapper/include/mapper_const.h"

namespace pos
{
VSAMapMock::VSAMapMock(TestInfo* testInfo)
: testInfo(testInfo)
{
    map = new VirtualBlkAddr*[testInfo->maxNumVolume];
    for (int idx = 0; idx < testInfo->maxNumVolume; idx++)
    {
        map[idx] = new VirtualBlkAddr[testInfo->maxVolumeSizeInBlock];
        for (uint64_t blk = 0; blk < testInfo->maxVolumeSizeInBlock; blk++)
        {
            map[idx][blk] = UNMAP_VSA;
        }
    }

    ON_CALL(*this, SetVSAsInternal).WillByDefault(::testing::Invoke(this,
        &VSAMapMock::_SetVSAsInternal));
    ON_CALL(*this, SetVSAsWithSyncOpen).WillByDefault(::testing::Invoke(this,
        &VSAMapMock::_SetVSAsWithSyncOpen));
}

VSAMapMock::~VSAMapMock(void)
{
    for (int idx = 0; idx < testInfo->maxNumVolume; idx++)
    {
        delete [] map[idx];
    }
    delete [] map;
}

VirtualBlkAddr
VSAMapMock::GetVSAInternal(int volumeId, BlkAddr rba, int& caller)
{
    assert(volumeId < testInfo->maxNumVolume);
    assert(rba < testInfo->maxVolumeSizeInBlock);

    caller = OK_READY;
    return map[volumeId][rba];
}

MpageList
VSAMapMock::GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks)
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
VSAMapMock::_SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    assert(volumeId < testInfo->maxNumVolume);

    for (uint32_t blkCount = 0; blkCount < virtualBlks.numBlks; blkCount++)
    {
        assert(startRba + blkCount < testInfo->maxVolumeSizeInBlock);
        map[volumeId][startRba + blkCount].stripeId = virtualBlks.startVsa.stripeId;
        map[volumeId][startRba + blkCount].offset = virtualBlks.startVsa.offset + blkCount;
    }
    return 0;
}

int
VSAMapMock::GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks,
    VsaArray& vsaArray)
{
    return 0;
}

int
VSAMapMock::SetVSAs(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
    return 0;
}

VirtualBlkAddr
VSAMapMock::GetRandomVSA(BlkAddr rba)
{
    return UNMAP_VSA;
}

int64_t
VSAMapMock::GetNumUsedBlks(int volId)
{
    return 0;
}

VirtualBlkAddr
VSAMapMock::GetVSAWithSyncOpen(int volId, BlkAddr rba)
{
    assert(volId < testInfo->maxNumVolume);
    assert(rba < testInfo->maxVolumeSizeInBlock);

    return map[volId][rba];
}

int
VSAMapMock::_SetVSAsWithSyncOpen(int volId, BlkAddr startRba, VirtualBlks& virtualBlks)
{
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
