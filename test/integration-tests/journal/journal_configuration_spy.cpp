#include "test/integration-tests/journal/journal_configuration_spy.h"

namespace pos
{
JournalConfigurationSpy::JournalConfigurationSpy(bool isJournalEnabled, uint64_t logBufferSize, uint64_t pageSize,
    uint64_t partitionSize, bool isRocksDBEnabled, std::string rocksDBBasePath, bool isVscEnabled)
: JournalConfiguration()
{
    journalEnabled = isJournalEnabled;
    logBufferSizeInConfig = logBufferSize;

    metaPageSize = pageSize;
    maxPartitionSize = partitionSize;
    rocksdbEnabled = isRocksDBEnabled;
    rocksdbPath = rocksDBBasePath;
    vscEnabled = isVscEnabled;
}

JournalConfigurationSpy::~JournalConfigurationSpy(void)
{
}

int
JournalConfigurationSpy::SetLogBufferSize(uint64_t loadedBufferSize, MetaFsFileControlApi* metaFsCtrl)
{
    if (loadedBufferSize == 0)
    {
        _SetLogBufferSize(logBufferSizeInConfig);
    }
    else
    {
        _SetLogBufferSize(loadedBufferSize);
    }

    return 0;
}

} // namespace pos
