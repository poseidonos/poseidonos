#include <gtest/gtest.h>
#include "src/array_components/array_mount_progress.h"

namespace pos
{
TEST(ArrayMountProgress, Get_testIfProgressIsReturnedToMinusOneBeforeInit)
{
    // Given
    ArrayMountProgress progress;
    int NO_ACTIVE_PROGRESS = -1;

    // When
    int ret = progress.Get(); // Get progress without init

    // Then
    ASSERT_EQ(NO_ACTIVE_PROGRESS, ret);
}

TEST(ArrayMountProgress, Get_testIfProgressIsReturnedToMinusOneBeforeSet)
{
    // Given
    ArrayMountProgress progress;
    int NO_ACTIVE_PROGRESS = -1;
    int totalProgress = 100; // not interesting
    string arrayName = "posarray"; // not interesting
    MountProgressType type = MountProgressType::MOUNT; // not interesting

    // When
    progress.Init(arrayName, type, totalProgress);
    int ret = progress.Get(); // Get progress without Set

    // Then
    ASSERT_EQ(NO_ACTIVE_PROGRESS, ret);
}

TEST(ArrayMountProgress, Get_testIfProgressIsReturnedZeroRightAfterSet)
{
    // Given
    ArrayMountProgress progress;
    int totalProgress = 100; // not interesting
    string arrayName = "posarray"; // not interesting
    MountProgressType type = MountProgressType::MOUNT; // not interesting

    // When
    progress.Init(arrayName, type, totalProgress);
    progress.Set();
    int ret = progress.Get(); // Get progress without Set

    // Then
    ASSERT_EQ(0, ret);
}

TEST(ArrayMountProgress, Get_testIfProgressIsReturnedToMinusOneAfterReset)
{
    // Given
    ArrayMountProgress progress;
    int NO_ACTIVE_PROGRESS = -1;
    int totalProgress = 100; // not interesting
    string arrayName = "posarray"; // not interesting
    MountProgressType type = MountProgressType::MOUNT; // not interesting

    // When
    progress.Init(arrayName, type, totalProgress);
    progress.Set();
    progress.Update(50); // not interesting
    progress.Reset();
    int ret = progress.Get(); // Get progress after reset

    // Then
    ASSERT_EQ(NO_ACTIVE_PROGRESS, ret);
}

TEST(ArrayMountProgress, Update_testIfProgressIsCalculatedWell)
{
    // Given
    ArrayMountProgress progress;
    int totalProgress = 100;
    int firstUpdate = 10;
    int progressRateAfterFirstUpdate = 10;
    int secondUpdate = 15;
    int progressRateAfterSecondUpdate = 25;
    int thirdUpdate = 75;
    int progressRateAfterThirdUpdate = 100;
    string arrayName = "posarray"; // not interesting
    MountProgressType type = MountProgressType::MOUNT; // not interesting

    // When
    progress.Init(arrayName, type, totalProgress);
    progress.Set();
    progress.Update(firstUpdate);
    ASSERT_EQ(progressRateAfterFirstUpdate, progress.Get());
    progress.Update(secondUpdate);
    ASSERT_EQ(progressRateAfterSecondUpdate, progress.Get());
    progress.Update(thirdUpdate);
    ASSERT_EQ(progressRateAfterThirdUpdate, progress.Get());
}

}  // namespace pos
