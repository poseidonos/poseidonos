#pragma once

#include <atomic>
#include <list>

#include "src/journal_manager/log/log_handler.h"
#include "src/journal_manager/log/log_list.h"
#include "src/journal_manager/log/gc_map_update_list.h"

#include "src/allocator/stripe/stripe.h"
#include "src/bio/volume_io.h"
#include "src/include/address_type.h"

namespace pos
{
class WrittenLogs
{
public:
    WrittenLogs(void);
    ~WrittenLogs(void);

    void Reset(void);

    // TODO (cheolho.kang) naming
    void AddToWriteList(VolumeIoSmartPtr volumeIo);
    void AddToWriteList(Stripe* stripe, StripeAddr oldAddr);
    void AddToWriteList(GcStripeMapUpdateList mapUpdates);

    bool CheckLogInTheList(LogHandlerInterface* log);

    void JournalWriteDone(void);
    void WaitForAllLogWriteDone(void);

    bool AreAllLogWritesDone(void);
    uint32_t GetNumLogsInTesting(void);

    void CompareLogList(LogList& input);

private:
    void _AddToList(LogHandlerInterface* entry);

    std::mutex logListLock;
    LogList writeLogList; // TODO (cheolho.kang) naming

    std::atomic<uint32_t> numJournalIssued;
    std::atomic<uint32_t> numJournalAdded;
};
} // namespace pos
