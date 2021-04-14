#include "test/integration-tests/journal/journal_configuration_spy.h"

namespace pos
{
JournalConfigurationSpy::JournalConfigurationSpy(bool isJournalEnabled, uint64_t logBufferSize, uint64_t pageSize, uint64_t partitionSize)
: JournalConfiguration("")
{
    journalEnabled = isJournalEnabled;
    logBufferSizeInConfig = logBufferSize;

    metaPageSize = pageSize;
    maxPartitionSize = partitionSize;
}

JournalConfigurationSpy::~JournalConfigurationSpy(void)
{
}

void
JournalConfigurationSpy::Init(void)
{
    _ConfigureLogBufferSize();
}

} // namespace pos
