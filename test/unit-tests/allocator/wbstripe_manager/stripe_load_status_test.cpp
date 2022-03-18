#include "src/allocator/wbstripe_manager/stripe_load_status.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(StripeLoadStatus, StripeLoadStarted_testIfValueIncreased)
{
    StripeLoadStatus status;

    status.Reset();
    EXPECT_EQ(status.GetNumStripesToLoad(), 0);
    EXPECT_EQ(status.GetNumStripesLoaded(), 0);
    EXPECT_EQ(status.GetNumStripesFailed(), 0);

    status.StripeLoadStarted();
    EXPECT_EQ(status.GetNumStripesToLoad(), 1);
    EXPECT_EQ(status.IsDone(), false);

    status.StripeLoadStarted();
    EXPECT_EQ(status.GetNumStripesToLoad(), 2);
    EXPECT_EQ(status.IsDone(), false);

    status.StripeLoadStarted();
    EXPECT_EQ(status.GetNumStripesToLoad(), 3);
    EXPECT_EQ(status.IsDone(), false);

    status.StripeLoaded();
    EXPECT_EQ(status.GetNumStripesLoaded(), 1);
    EXPECT_EQ(status.IsDone(), false);

    status.StripeLoaded();
    EXPECT_EQ(status.GetNumStripesLoaded(), 2);
    EXPECT_EQ(status.IsDone(), false);

    status.StripeLoaded();
    EXPECT_EQ(status.GetNumStripesLoaded(), 3);
    EXPECT_EQ(status.IsDone(), true);

    EXPECT_EQ(status.GetNumStripesToLoad(), 3);
    EXPECT_EQ(status.GetNumStripesLoaded(), 3);
    EXPECT_EQ(status.GetNumStripesFailed(), 0);
    status.Reset();

    EXPECT_EQ(status.GetNumStripesToLoad(), 0);
    EXPECT_EQ(status.GetNumStripesLoaded(), 0);
    EXPECT_EQ(status.GetNumStripesFailed(), 0);
}

} // namespace pos
