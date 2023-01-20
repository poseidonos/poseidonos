#include "test/integration-tests/journal/fake/test_journal_write_completion_with_meta_update.h"

#include "src/journal_manager/log_buffer/i_versioned_segment_context.h"
#include "test/integration-tests/journal/utils/test_info.h"
#include "test/integration-tests/journal/utils/written_logs.h"

namespace pos
{
TestJournalWriteCompletionWithMetaUpdate::TestJournalWriteCompletionWithMetaUpdate(WrittenLogs* logs, IVersionedSegmentContext* versionedContext, TestInfo* testInfo, VirtualBlks blks, LogEventType eventType)
: logs(logs),
  versionedContext(versionedContext),
  testInfo(testInfo),
  blks(blks),
  eventType(eventType)
{
}

bool
TestJournalWriteCompletionWithMetaUpdate::_DoSpecificJob(void)
{
    SegmentId segmentId = blks.startVsa.stripeId / testInfo->numStripesPerSegment;
    switch (eventType)
    {
        // Validate만 있는 replay 시나리오 한정으로 임시작성한 것이라 invalidate 처리와, segment full에 대한 코드는 아직 작성하지 않았습니다. 
        case LogEventType::BLOCK_MAP_UPDATE:
            versionedContext->IncreaseValidBlockCount(logGroupId, segmentId, blks.numBlks);
            break;
        case LogEventType::STRIPE_MAP_UPDATE:
            versionedContext->IncreaseOccupiedStripeCount(logGroupId, segmentId);
            break;
        default:
            break;
    }
    logs->JournalWriteDone();
    return true;
}
} // namespace pos
