#include "src/journal_manager/replay/task_progress.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(TaskProgress, TaskProgress_testIfConstructedSuccessfully)
{
    TaskProgress defaultProgress;
    TaskProgress progressWithWeight(5);

    TaskProgress* progress = new TaskProgress();
    delete progress;
}

TEST(TaskProgress, Start_testIfNumSubTaskUpdated)
{
    TaskProgress progress(100);
    progress.Start(5);

    EXPECT_EQ(progress.GetNumSubTasks(), 5);
}

TEST(TaskProgress, SubTaskCompleted_testIfAllSubTaskCompleted)
{
    TaskProgress progress(100);

    int numSubTasks = 5;
    progress.Start(numSubTasks);

    progress.SubTaskCompleted(1);
    EXPECT_EQ(progress.GetNumCompletedSubTasks(), 1);

    progress.SubTaskCompleted(3);
    EXPECT_EQ(progress.GetNumCompletedSubTasks(), 4);

    progress.SubTaskCompleted(2);
    EXPECT_EQ(progress.GetNumCompletedSubTasks(), numSubTasks);
}

TEST(TaskProgress, Complete_testIfAllSubtaskCompleted)
{
    TaskProgress progress(100);

    int numSubTasks = 5;
    progress.Start(numSubTasks);

    progress.Complete();
    EXPECT_EQ(progress.GetNumCompletedSubTasks(), numSubTasks);
}

TEST(TaskProgress, GetCurerntProgress_testIfProgressIsCalculatedCorrectly)
{
    TaskProgress progress(100);

    int numSubTasks = 5;
    progress.Start(numSubTasks);

    EXPECT_EQ(progress.GetCurerntProgress(), 0);

    progress.SubTaskCompleted(1);
    EXPECT_EQ(progress.GetCurerntProgress(), 100 * 1 / 5);

    progress.SubTaskCompleted(2);
    EXPECT_EQ(progress.GetCurerntProgress(), 100 * 3 / 5);

    progress.Complete();
    EXPECT_EQ(progress.GetCurerntProgress(), 100);
}

TEST(TaskProgress, GetWeight_testIfCorrectWeightIsReturned)
{
    TaskProgress progress(45);
    EXPECT_EQ(progress.GetWeight(), 45);
}
} // namespace pos
