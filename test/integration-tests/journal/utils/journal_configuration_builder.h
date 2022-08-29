#pragma once

#include "test/integration-tests/journal/journal_configuration_spy.h"
#include "test/integration-tests/journal/utils/test_info.h"

namespace pos
{
class JournalConfigurationBuilder
{
public:
    explicit JournalConfigurationBuilder(TestInfo* testInfo);
    virtual ~JournalConfigurationBuilder(void);

    JournalConfigurationBuilder* SetJournalEnable(bool isJournalEnabled);
    JournalConfigurationBuilder* SetLogBufferSize(uint64_t logBufferSize);
    JournalConfigurationBuilder* SetMetaPageSize(uint64_t metaPageSize);
    JournalConfigurationBuilder* SetMaxPartitionSize(uint64_t partitionSize);
    JournalConfigurationBuilder* SetRocksDBEnable(uint64_t isRocksDBEnabled);
    JournalConfigurationBuilder* SetRocksDBBasePath(std::string rocksDBBasePath);
    JournalConfigurationSpy* Build(void);

private:
    bool isJournalEnabled;
    uint64_t logBufferSize;
    uint64_t metaPageSize;
    uint64_t partitionSize;
    bool isRocksDBEnabled;
    std::string rocksDBBasePath;
    bool isVscEnabled;
};
} // namespace pos
