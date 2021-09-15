#include "src/lib/counter_timeout_checker.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(CounterTimeoutChecker, CounterTimeoutChecker_)
{
    //When: Create target object in stack
    CounterTimeoutChecker ctc;

    //Then: Do nothing

    //When: Create target object in Heap
    CounterTimeoutChecker* pCtc = new CounterTimeoutChecker();
    delete pCtc;

    //Then: Do nothing
}

TEST(CounterTimeoutChecker, SetTimeout_)
{
    //Given
    CounterTimeoutChecker ctc;

    //When
    ctc.SetTimeout(1);

    //Then: Do nothing
}

TEST(CounterTimeoutChecker, CheckTimeout_)
{
    //Given
    CounterTimeoutChecker ctc;

    //When: check default timeout status
    //Then: timeout should occur since remaining count is 0
    EXPECT_TRUE(ctc.CheckTimeout());

    //When: set timeout count to 2
    ctc.SetTimeout(2);

    //Then: timeout should not occur (remaining count is 2)
    EXPECT_FALSE(ctc.CheckTimeout());

    //Then: timeout should not occur (remaining count is 1)
    EXPECT_FALSE(ctc.CheckTimeout());

    //Then: timeout should occur (remaining count is 0)
    EXPECT_TRUE(ctc.CheckTimeout());
}

} // namespace pos
