#include "test/integration-tests/journal/utils/rba_generator.h"

#include <assert.h>

namespace pos
{
RbaGenerator::RbaGenerator(TestInfo* _testInfo)
{
    testInfo = _testInfo;
    currentAddr = 0;
}

RbaGenerator::~RbaGenerator(void)
{
}

BlkAddr
RbaGenerator::Generate(uint32_t numBlks)
{
    std::lock_guard<std::mutex> lock(generateLock);

    BlkAddr rba = currentAddr;
    currentAddr += numBlks;

    if (currentAddr >= testInfo->defaultTestVolSizeInBlock)
    {
        currentAddr = 0;
    }

    assert(currentAddr < testInfo->defaultTestVolSizeInBlock);

    return rba;
}
} // namespace pos
