#include "test_journal_write_completion.h"

#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"
#include "test/integration-tests/journal/fake/i_context_manager_fake.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/written_logs.h"
#include "src/include/address_type.h"

namespace pos
{
TestJournalWriteCompletion::TestJournalWriteCompletion(WrittenLogs* logs, IContextManagerFake* contextManager, TestInfo* testInfo, VirtualBlks blks, LogType eventType)
: logs(logs),
  contextManager(contextManager),
  testInfo(testInfo),
  blks(blks),
  eventType(eventType)
{
    segmentCtx = contextManager->GetSegmentContextUpdaterPtr();
    versionedSegCtx = contextManager->GetVersionedSegmentContext();
}

bool
TestJournalWriteCompletion::_DoSpecificJob(void)
{
    SegmentId segmentId = blks.startVsa.stripeId / testInfo->numStripesPerSegment;
    switch (eventType)
    {
        case LogType::BLOCK_WRITE_DONE:
            // TODO (cheolho.kang): Add a process to check RBA to detect if it's an overwrite and then invalidate it. Need to implement this logic in test_fixture
            segmentCtx->ValidateBlks(blks);
            versionedSegCtx->IncreaseValidBlockCount(logGroupId, segmentId, blks.numBlks);
            break;
        case LogType::STRIPE_MAP_UPDATED:
            StripeId vsid = VsidToUserLsid(blks.startVsa.stripeId);
            segmentCtx->UpdateOccupiedStripeCount(vsid);
            versionedSegCtx->IncreaseOccupiedStripeCount(logGroupId, segmentId);
            break;
        case LogType::GC_STRIPE_FLUSHED:
            // TODO (cheolho.kang): Implement later
            break;
        default:
            break;
    }
    logs->JournalWriteDone();
    return true;
}
} // namespace pos
