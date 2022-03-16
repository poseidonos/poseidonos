#include "src/journal_manager/replay/flush_pending_stripes.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/journal_manager/replay/pending_stripe_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushPendingStripes, Start_testIfTaskCompletedSuccessfullyWhenThereIsOnlyOnePendingStripe)
{
    // Given
    NiceMock<PendingStripe>* pendingStripe = new NiceMock<PendingStripe>;
    PendingStripeList pendingStripeList(1, pendingStripe);
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // Then
    EXPECT_CALL(wbStripeAllocator, FinishStripe).Times(1);

    // When
    int result = flushPendingStripesTask.Start();

    // Then
    EXPECT_EQ(result, 0);
}

TEST(FlushPendingStripes, Start_testIfTaskCompletedSuccessfullyWhenThereArePendingStripes)
{
    // Given
    int numPendingStripes = 5;
    PendingStripeList pendingStripeList;
    for (int count = 0; count < numPendingStripes; count++)
    {
        pendingStripeList.push_back(new NiceMock<PendingStripe>);
    }
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // Then
    EXPECT_CALL(wbStripeAllocator, FinishStripe).Times(numPendingStripes);
    EXPECT_CALL(wbStripeAllocator, FlushAllPendingStripes).Times(1);

    // When
    int result = flushPendingStripesTask.Start();

    // Then
    EXPECT_EQ(result, 0);
}

TEST(FlushPendingStripes, GetId_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<PendingStripe>* pendingStripe = new NiceMock<PendingStripe>;
    PendingStripeList pendingStripeList(1, pendingStripe);
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // When
    ReplayTaskId taskId = flushPendingStripesTask.GetId();

    // Then
    EXPECT_EQ(taskId, ReplayTaskId::FLUSH_PENDING_STRIPES);
}

TEST(FlushPendingStripes, GetWeight_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<PendingStripe>* pendingStripe = new NiceMock<PendingStripe>;
    PendingStripeList pendingStripeList(1, pendingStripe);
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // When
    int weight = flushPendingStripesTask.GetWeight();

    // Then: Executed Successfully without any error
}

TEST(FlushPendingStripes, GetNumSubTasks_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<PendingStripe>* pendingStripe = new NiceMock<PendingStripe>;
    PendingStripeList pendingStripeList(1, pendingStripe);
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // When
    int numSubTasks = flushPendingStripesTask.GetNumSubTasks();

    // Then: Executed Successfully without any error
}

} // namespace pos
