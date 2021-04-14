#pragma once

#include <mutex>

#include "test/integration-tests/journal/utils/test_info.h"
#include "src/include/address_type.h"

namespace pos
{
class RbaGenerator
{
public:
    explicit RbaGenerator(TestInfo* _testInfo);
    virtual ~RbaGenerator(void);

    BlkAddr Generate(uint32_t numBlks);

private:
    TestInfo* testInfo;

    std::mutex generateLock;
    BlkAddr currentAddr;
};
} // namespace pos
