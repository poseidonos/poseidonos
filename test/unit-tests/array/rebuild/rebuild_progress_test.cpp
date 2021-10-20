#include "src/array/rebuild/rebuild_progress.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(RebuildProgress, RebuildProgress_testConstructor)
{
    // Given: nothing

    // When
    RebuildProgress *rp = new RebuildProgress("mock-array");

    // Then
    delete rp;
}

TEST(RebuildProgress, Update_testIfFirstAndSecondProgressWorksAsExpected)
{
    // Given
    RebuildProgress rp("mock-array");
    string metaPartId = "MetaPartition";
    string dataPartId = "DataPartition";

    // When 1: init progress
    rp.Update(metaPartId, 0, 1024*2);
    rp.Update(dataPartId, 0, 1024*98);

    // Then 1: the percent should match with the initial value
    ASSERT_EQ(0, rp.Current());

    // When 2: progress updated
    rp.Update(metaPartId, 1024, 1024*2);

    // Then 2: total progress will be 1%
    ASSERT_EQ(1, rp.Current());

    // When 3: progress updated
    rp.Update(metaPartId, 1024*2, 1024*2);

    // Then 3: total progress will be 2%
    ASSERT_EQ(2, rp.Current());

    // When 4: progress updated - target segments are reduced
    rp.Update(dataPartId, 0, 1024*90);

    // Then 4: total progress will be 10%
    ASSERT_EQ(10, rp.Current());


    // When 5: progress updated - all segments are rebuilt. no remaining targets.
    rp.Update(dataPartId, 0, 0);

    // Then 5: total progress will be 100%
    ASSERT_EQ(100, rp.Current());
}

} // namespace pos
