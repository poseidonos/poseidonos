#pragma once

#include "test/integration-tests/journal/journal_manager_spy.h"

namespace pos
{
class UsedOffsetCalculator
{
public:
    UsedOffsetCalculator(JournalManagerSpy* journal, uint64_t sizeToFill);
    bool CanBeWritten(uint32_t logSize);

private:
    uint64_t _GetNextOffset(uint32_t logSize);

    JournalManagerSpy* journal;

    uint64_t sizeToFill;
    uint64_t sizeUsed;

    uint64_t startOffset;
    uint64_t currentOffset;
};
} // namespace pos
