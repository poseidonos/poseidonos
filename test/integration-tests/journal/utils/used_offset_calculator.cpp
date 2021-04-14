#include "test/integration-tests/journal/utils/used_offset_calculator.h"

namespace pos
{
UsedOffsetCalculator::UsedOffsetCalculator(JournalManagerSpy* journal,
    uint64_t sizeToFill)
: journal(journal),
  sizeToFill(sizeToFill),
  sizeUsed(0)
{
    startOffset = journal->GetNextOffset();
    currentOffset = startOffset;
}

bool
UsedOffsetCalculator::CanBeWritten(uint32_t logSize)
{
    uint64_t previousOffset = currentOffset;
    currentOffset = journal->GetNextOffset();

    if (previousOffset <= currentOffset)
    {
        sizeUsed += (currentOffset - previousOffset);
    }
    else
    {
        sizeUsed += (journal->GetLogBufferSize() - previousOffset + currentOffset);
    }

    return (sizeUsed + logSize <= sizeToFill);
}
} // namespace pos
