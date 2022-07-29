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
    currentOffset = _GetNextOffset(logSize);

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

uint64_t
UsedOffsetCalculator::_GetNextOffset(uint32_t logSize)
{
    uint64_t nextOffset = journal->GetNextOffset();

    uint64_t metaPageSize = journal->GetMetaPageSize();
    uint64_t currentMetaPage = nextOffset / metaPageSize;
    uint64_t endMetaPage = (nextOffset + logSize - 1) / metaPageSize;

    if (currentMetaPage != endMetaPage)
    {
        nextOffset = endMetaPage * metaPageSize;
    }

    return nextOffset;
}
} // namespace pos
