#include "src/qos/rate_limit.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(BwIopsRateLimit, BwIopsRateLimit_Constructor_One_Stack)
{
    BwIopsRateLimit bwIopsRateLimit();
}

TEST(BwIopsRateLimit, BwIopsRateLimit_Constructor_One_Heap)
{
    BwIopsRateLimit* bwIopsRateLimit = new BwIopsRateLimit();
    delete bwIopsRateLimit;
}

TEST(BwIopsRateLimit, IsLimitExceeded_Initial_Case)
{
    BwIopsRateLimit bwIopsRateLimit;
    uint32_t id1 = 0;
    uint32_t id2 = 0;
    bool expectedRateExceeded = false;
    // since intial case  iopsRateLimit[id1][id2] <= 0,condition wont satisy so expected value is false
    bool actualRateExceeded;
    actualRateExceeded = bwIopsRateLimit.IsLimitExceeded(id1, id2);
    ASSERT_EQ(expectedRateExceeded, actualRateExceeded);
}

TEST(BwIopsRateLimit, ResetRateLimit_Check_Setter_bwRateLimit)
{
    BwIopsRateLimit bwIopsRateLimit;

    uint32_t id1 = 0;
    uint32_t id2 = 0;
    double offset = -2;
    const int64_t bwLimit = DEFAULT_MAX_BW_IOPS + 1;
    const int64_t iopsLimit = 0;
    bwIopsRateLimit.ResetRateLimit(id1, id2, offset, bwLimit, iopsLimit);
    bool expectedRateExceeded = true;
    // since Reset Rate Limit will set bwRateLimit[id1][id2] <= 0,condition will satisy so expected value is true
    bool actualRateExceeded;
    actualRateExceeded = bwIopsRateLimit.IsLimitExceeded(id1, id2);
    ASSERT_EQ(expectedRateExceeded, actualRateExceeded);
}
TEST(BwIopsRateLimit, UpdateRateLimit_Set_and_Check_for_Negative_bwRateLimit)
{
    BwIopsRateLimit bwIopsRateLimit;

    uint32_t id1 = 0;
    uint32_t id2 = 0;
    // using this value to make bwVal negative so that it can be tested using IsLimitExceeded function
    uint64_t bwVal = DEFAULT_MAX_BW_IOPS + 1;
    bwIopsRateLimit.UpdateRateLimit(id1, id2, bwVal);

    // since bwIopsRateLimit will set the bw to negative to return should be true
    bool expectedRateExceeded = true;
    bool actualRateExceeded;
    actualRateExceeded = bwIopsRateLimit.IsLimitExceeded(id1, id2);
    ASSERT_EQ(expectedRateExceeded, actualRateExceeded);
}

TEST(BwIopsRateLimit, IsLimitExceeded_Bw)
{
    BwIopsRateLimit bwIopsRateLimit;
    uint32_t id1 = 0;
    uint32_t id2 = 0;
    bwIopsRateLimit.ResetRateLimit(id1, id2, 0, 0, 0);
    bwIopsRateLimit.UpdateRateLimit(id1, id2, 1);
    bool expectedRateExceeded = true;
    bool actualRateExceeded = bwIopsRateLimit.IsLimitExceeded(id1, id2);
    ASSERT_EQ(expectedRateExceeded, actualRateExceeded);
}

} // namespace pos
