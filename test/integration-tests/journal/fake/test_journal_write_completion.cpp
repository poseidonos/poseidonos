#include "test_journal_write_completion.h"

#include "src/allocator/i_segment_ctx.h"
#include "src/include/address_type.h"
#include "test/integration-tests/journal/fake/vsamap_fake.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/written_logs.h"

namespace pos
{
TestJournalWriteCompletion::TestJournalWriteCompletion(VolumeIoSmartPtr volumeIo, SegmentContextUpdater* segmentContextUpdater, VSAMapFake* vsaMap, WrittenLogs* logs, LogType eventType)
: volumeIo(volumeIo),
  segmentContextUpdater(segmentContextUpdater),
  vsaMap(vsaMap),
  logs(logs),
  eventType(eventType)
{
}

bool
TestJournalWriteCompletion::_DoSpecificJob(void)
{
    if (eventType == LogType::BLOCK_WRITE_DONE)
    {
        _InvalidateOldBlock();
        _UpdateBlockMap();
    }
    else if (eventType == LogType::STRIPE_MAP_UPDATED)
    {
        StripeId lsid = volumeIo->GetUserLsid();
        segmentContextUpdater->UpdateOccupiedStripeCountWithGroupId(lsid, logGroupId);
    }
    else if (eventType == LogType::GC_STRIPE_FLUSHED)
    {
    }
    logs->JournalWriteDone();
    return true;
}

void
TestJournalWriteCompletion::_InvalidateOldBlock(void)
{
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    for (uint32_t blkCount = 0; blkCount < blockCount; blkCount++)
    {
        VirtualBlkAddr oldVsa = vsaMap->GetVSAWithSyncOpen(volumeIo->GetVolumeId(), rba);
        if (!(oldVsa == UNMAP_VSA))
        {
            VirtualBlks oldBlock = {
                .startVsa = oldVsa,
                .numBlks = 1};
            segmentContextUpdater->InvalidateBlocksWithGroupId(oldBlock, false, logGroupId);
        }
    }
}

void
TestJournalWriteCompletion::_UpdateBlockMap(void)
{
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlks targetVsaRange = {
        .startVsa = volumeIo->GetVsa(),
        .numBlks = blockCount};
    vsaMap->SetVSAsWithSyncOpen(volumeIo->GetVolumeId(), rba, targetVsaRange);
    segmentContextUpdater->ValidateBlocksWithGroupId(targetVsaRange, logGroupId);
}
} // namespace pos
