#include "test/integration-tests/journal/utils/journal_configuration_builder.h"

#include "src/logger/logger.h"

namespace pos
{
JournalConfigurationBuilder::JournalConfigurationBuilder(TestInfo* testInfo)
: isJournalEnabled(true),
  logBufferSize(16 * 1024 * 1024),
  metaPageSize(testInfo->metaPageSize),
  partitionSize(testInfo->metaPartitionSize)
{
}

JournalConfigurationBuilder::~JournalConfigurationBuilder(void)
{
}

JournalConfigurationBuilder*
JournalConfigurationBuilder::SetJournalEnable(bool isJournalEnabled)
{
    this->isJournalEnabled = isJournalEnabled;
    return this;
}

JournalConfigurationBuilder*
JournalConfigurationBuilder::SetLogBufferSize(uint64_t logBufferSize)
{
    this->logBufferSize = logBufferSize;
    return this;
}

JournalConfigurationBuilder*
JournalConfigurationBuilder::SetMetaPageSize(uint64_t metaPageSize)
{
    this->metaPageSize = metaPageSize;
    return this;
}

JournalConfigurationBuilder*
JournalConfigurationBuilder::SetMaxPartitionSize(uint64_t partitionSize)
{
    this->partitionSize = partitionSize;
    return this;
}

JournalConfigurationSpy*
JournalConfigurationBuilder::Build(void)
{
    JournalConfigurationSpy* config = new JournalConfigurationSpy(isJournalEnabled, logBufferSize, metaPageSize, partitionSize);
    return config;
}

} // namespace pos
