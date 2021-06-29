#include "src/array/rebuild/rebuild_progress.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(RebuildProgress, RebuildProgress_testConstructor)
{
    // Given: nothing

    // When
    RebuildProgress rp("mock-array");

    // Then
}

TEST(RebuildProgress, Update_testIfFirstAndSecondProgressWorksAsExpected)
{
    // Given
    RebuildProgress rp("mock-array");
    rp.SetTotal(100);
    string rebuildId = "rebuildId1";

    // When 1: first progress report
    rp.Update(rebuildId, 10);

    // Then 1: the percent should match with the initial value
    ASSERT_EQ(10, rp.Current());

    // When 2: second progress report
    rp.Update(rebuildId, 30);

    // Then 2: the percent should be accumulated based on the delta
    ASSERT_EQ(30, rp.Current());

    // TODO(srm): please check whether RebuildProgress is intended to use only a single id. (When 3 & Then 3) doesn't work.
    // When 3: first progress report with different id
    // rp.Update("differentId", 40);

    // Then 3: the percent should match with the initial value
    // ASSERT_EQ(40, rp.Current());
}

} // namespace pos
