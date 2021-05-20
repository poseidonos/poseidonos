#pragma once

#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
class JournalConfigurationSpy : public JournalConfiguration
{
public:
    JournalConfigurationSpy(bool isJournalEnabled, uint64_t logBufferSize, uint64_t pageSize, uint64_t partitionSize);
    virtual ~JournalConfigurationSpy(void);

    virtual int Init(uint64_t logBufferSize = 0) override;
};
} // namespace pos
