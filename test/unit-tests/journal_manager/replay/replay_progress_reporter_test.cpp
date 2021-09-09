#include "src/journal_manager/replay/replay_progress_reporter.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ReplayProgressReporter, ReplayProgressReporter_testIfConstructedProperly)
{
    ReplayProgressReporter reporter;

    EXPECT_EQ(reporter.GetProgress(), 0);
}

TEST(ReplayProgressReporter, RegisterTask_testIfTaskRegistered)
{
    ReplayProgressReporter reporter;
    reporter.RegisterTask(ReplayTaskId::READ_LOG_BUFFER, 10);
    reporter.RegisterTask(ReplayTaskId::REPLAY_LOGS, 30);
    reporter.RegisterTask(ReplayTaskId::REPLAY_VOLUME_DELETION, 60);

    TaskProgress task1 = reporter.GetTaskProgress(ReplayTaskId::READ_LOG_BUFFER);
    EXPECT_EQ(task1.GetWeight(), 10);

    TaskProgress task2 = reporter.GetTaskProgress(ReplayTaskId::REPLAY_LOGS);
    EXPECT_EQ(task2.GetWeight(), 30);

    TaskProgress task3 = reporter.GetTaskProgress(ReplayTaskId::REPLAY_VOLUME_DELETION);
    EXPECT_EQ(task3.GetWeight(), 60);

    EXPECT_EQ(reporter.GetTotalWeight(), 10 + 30 + 60);
}

TEST(ReplayProgressReporter, testIfTaskProgressUpdatedProperly)
{
    ReplayProgressReporter reporter;
    reporter.RegisterTask(ReplayTaskId::READ_LOG_BUFFER, 10);
    reporter.RegisterTask(ReplayTaskId::REPLAY_LOGS, 50);
    reporter.RegisterTask(ReplayTaskId::REPLAY_VOLUME_DELETION, 10);
    reporter.RegisterTask(ReplayTaskId::FLUSH_METADATA, 10);
    reporter.RegisterTask(ReplayTaskId::RESET_LOG_BUFFER, 10);
    reporter.RegisterTask(ReplayTaskId::FLUSH_PENDING_STRIPES, 10);

    int reported = 0;

    reporter.TaskStarted(ReplayTaskId::READ_LOG_BUFFER, 1);
    reporter.SubTaskCompleted(ReplayTaskId::READ_LOG_BUFFER);

    EXPECT_TRUE(reporter.GetReportedProgress() > reported);
    reported = reporter.GetReportedProgress();

    reporter.TaskCompleted(ReplayTaskId::READ_LOG_BUFFER);

    EXPECT_TRUE(reporter.GetReportedProgress() >= reported);
    reported = reporter.GetReportedProgress();

    reporter.TaskStarted(ReplayTaskId::REPLAY_LOGS, 3);
    reporter.SubTaskCompleted(ReplayTaskId::REPLAY_LOGS);

    EXPECT_TRUE(reporter.GetReportedProgress() > reported);
    reported = reporter.GetReportedProgress();

    reporter.TaskCompleted(ReplayTaskId::REPLAY_LOGS);

    EXPECT_TRUE(reporter.GetReportedProgress() > reported);
    reported = reporter.GetReportedProgress();

    reporter.CompleteAll();
    EXPECT_EQ(reporter.GetReportedProgress(), 100);
}

} // namespace pos
