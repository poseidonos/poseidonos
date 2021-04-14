#include "test/integration-tests/journal/fake/test_journal_write_completion.h"
#include "test/integration-tests/journal/utils/written_logs.h"

namespace pos
{
TestJournalWriteCompletion::TestJournalWriteCompletion(WrittenLogs* writtenLogs)
{
    logs = writtenLogs;
}

bool
TestJournalWriteCompletion::Execute(void)
{
    logs->JournalWriteDone();
    return true;
}
} // namespace pos
