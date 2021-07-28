#include "src/journal_manager/journaling_status.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(JournalingStatus, Get_testIfReturnsCorrectly)
{
    JournalingStatus status;
    EXPECT_EQ(status.Get(), JOURNAL_INVALID);

    status.Set(WAITING_TO_BE_REPLAYED);
    EXPECT_EQ(status.Get(), WAITING_TO_BE_REPLAYED);

    status.Set(REPLAYING_JOURNAL);
    EXPECT_EQ(status.Get(), REPLAYING_JOURNAL);

    status.Set(JOURNALING);
    EXPECT_EQ(status.Get(), JOURNALING);
}

} // namespace pos
