#include "test/integration-tests/journal/utils/written_logs.h"

#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
namespace pos
{
WrittenLogs::WrittenLogs(void)
{
    numJournalIssued = 0;
    numJournalAdded = 0;
}

WrittenLogs::~WrittenLogs(void)
{
    Reset();
}

void
WrittenLogs::Reset(void)
{
    writeLogList.Reset();
}

void
WrittenLogs::_AddToList(LogHandlerInterface* entry)
{
    std::lock_guard<std::mutex> lock(logListLock);
    writeLogList.AddLog(0, entry); // logGroupId here is not used
}

void
WrittenLogs::AddToWriteList(VolumeIoSmartPtr volumeIo)
{
    numJournalIssued++;

    int volumeId = volumeIo->GetVolumeId();
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetSectorRba());
    uint64_t numBlks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    StripeAddr writeBufferStripe = volumeIo->GetLsidEntry();
    LogHandlerInterface* entry = new BlockWriteDoneLogHandler(volumeId,
        rba, numBlks, startVsa, volumeId, writeBufferStripe);

    _AddToList(entry);
}

void
WrittenLogs::AddToWriteList(Stripe* stripe, StripeAddr oldAddr)
{
    numJournalIssued++;

    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe->GetUserLsid()};
    LogHandlerInterface* entry = new StripeMapUpdatedLogHandler(stripe->GetVsid(), oldAddr, newAddr);

    _AddToList(entry);
}

void
WrittenLogs::AddToWriteList(GcStripeMapUpdateList mapUpdates)
{
    numJournalIssued++;

    LogHandlerInterface* blockLog = new GcBlockWriteDoneLogHandler(mapUpdates.volumeId,
        mapUpdates.vsid, mapUpdates.blockMapUpdateList);
    _AddToList(blockLog);

    LogHandlerInterface* stripeLog = new GcStripeFlushedLogHandler(mapUpdates.volumeId, mapUpdates.vsid,
        mapUpdates.wbLsid, mapUpdates.userLsid, mapUpdates.blockMapUpdateList.size());
    _AddToList(stripeLog);
}

bool
WrittenLogs::CheckLogInTheList(LogHandlerInterface* log)
{
    bool exist = false;
    auto logList = writeLogList.GetLogs();
    for (auto it = logList.begin(); it != logList.end(); it++)
    {
        if ((*it)->GetType() == log->GetType())
        {
            if (log->GetType() == LogType::BLOCK_WRITE_DONE)
            {
                BlockWriteDoneLogHandler* cmp1 = reinterpret_cast<BlockWriteDoneLogHandler*>(*it);
                BlockWriteDoneLogHandler* cmp2 = reinterpret_cast<BlockWriteDoneLogHandler*>(log);

                exist = (*cmp1 == *cmp2);
            }
            else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
            {
                StripeMapUpdatedLogHandler* cmp1 = reinterpret_cast<StripeMapUpdatedLogHandler*>(*it);
                StripeMapUpdatedLogHandler* cmp2 = reinterpret_cast<StripeMapUpdatedLogHandler*>(log);

                exist = (*cmp1 == *cmp2);
            }
            else if (log->GetType() == LogType::GC_BLOCK_WRITE_DONE)
            {
                GcBlockWriteDoneLogHandler cmp1((*it)->GetData());
                GcBlockWriteDoneLogHandler cmp2(log->GetData());

                exist = (cmp1 == cmp2);
            }
            else if (log->GetType() == LogType::GC_STRIPE_FLUSHED)
            {
                GcStripeFlushedLogHandler* cmp1 = reinterpret_cast<GcStripeFlushedLogHandler*>(*it);
                GcStripeFlushedLogHandler* cmp2 = reinterpret_cast<GcStripeFlushedLogHandler*>(log);

                exist = (*cmp1 == *cmp2);
            }
            else if (log->GetType() == LogType::VOLUME_DELETED)
            {
                VolumeDeletedLogEntry* cmp1 = reinterpret_cast<VolumeDeletedLogEntry*>(*it);
                VolumeDeletedLogEntry* cmp2 = reinterpret_cast<VolumeDeletedLogEntry*>(log);

                exist = (*cmp1 == *cmp2);
            }

            if (exist == true)
            {
                break;
            }
        }
    }
    return exist;
}

void
WrittenLogs::JournalWriteDone(void)
{
    numJournalAdded++;
}

void
WrittenLogs::WaitForAllLogWriteDone(void)
{
    while (numJournalIssued != numJournalAdded)
    {
    }
}

bool
WrittenLogs::AreAllLogWritesDone(void)
{
    return (numJournalIssued == numJournalAdded);
}

uint32_t
WrittenLogs::GetNumLogsInTesting(void)
{
    return writeLogList.GetLogs().size();
}
} // namespace pos
