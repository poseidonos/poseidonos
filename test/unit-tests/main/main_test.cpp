#include <gtest/gtest.h>

extern void INThandler(int sig);

namespace pos
{
TEST(Main, INTHandler)
{
    //Given: handler for SIGINT is default
    EXPECT_EQ(signal(SIGINT, NULL), SIG_DFL);

    //When: SIGINT occurs
    INThandler(SIGINT);

    //Then: following SIGINT shall be ignored
    EXPECT_EQ(signal(SIGINT, NULL), SIG_IGN);
}

} // namespace pos
