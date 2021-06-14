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
    EXPECT_CALL(wbStripeAllocator, ReconstructActiveStripe).WillOnce(Return(0));
    EXPECT_CALL(wbStripeAllocator, FinishReconstructedStripe).Times(1);

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
    EXPECT_CALL(wbStripeAllocator, ReconstructActiveStripe).WillRepeatedly(Return(0));
    EXPECT_CALL(wbStripeAllocator, FinishReconstructedStripe).Times(numPendingStripes);

    // When
    int result = flushPendingStripesTask.Start();

    // Then
    EXPECT_EQ(result, 0);
}

TEST(FlushPendingStripes, Start_testIfTaskCompletedWithNegativeValueWhenOneStripeReconstructionFails)
{
    // Given
    int numPendingStripes = 3;
    PendingStripeList pendingStripeList;
    for (int count = 0; count < numPendingStripes; count++)
    {
        pendingStripeList.push_back(new NiceMock<PendingStripe>);
    }
    NiceMock<MockIWBStripeAllocator> wbStripeAllocator;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushPendingStripes flushPendingStripesTask(pendingStripeList, &wbStripeAllocator, &reporter);

    // Then: 2/3 stripes success
    int retCode = -1000;
    EXPECT_CALL(wbStripeAllocator, ReconstructActiveStripe)
        .WillOnce(Return(0))
        .WillOnce(Return(retCode))
        .WillOnce(Return(0));
    EXPECT_CALL(wbStripeAllocator, FinishReconstructedStripe).Times(2);

    // When
    int result = flushPendingStripesTask.Start();

    // Then
    EXPECT_EQ(result, retCode);
}

TEST(FlushPendingStripes, Start_testIfTaskCompletedWithNegativeValueWhenSomeStripeReconstructionFails)
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

    // Then: 3/5 stripes success
    EXPECT_CALL(wbStripeAllocator, ReconstructActiveStripe).WillOnce(Return(0)).WillOnce(Return(-1000)).WillOnce(Return(0)).WillOnce(Return(-2000)).WillOnce(Return(0));
    EXPECT_CALL(wbStripeAllocator, FinishReconstructedStripe).Times(3);

    // When
    int result = flushPendingStripesTask.Start();

    // Then
    EXPECT_EQ(result, -1000);
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
