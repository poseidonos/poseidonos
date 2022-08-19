#include "src/rebuild/rebuild_completed.h"
#include "test/unit-tests/rebuild/rebuild_behavior_mock.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(RebuildCompleted, RebuildCompleted_testConstructor)
{
    // Given
    MockRebuildBehavior* rb = new MockRebuildBehavior(nullptr);
    // When
    RebuildCompleted rc(rb);
}

TEST(RebuildCompleted, Execute_)
{
}

} // namespace pos
