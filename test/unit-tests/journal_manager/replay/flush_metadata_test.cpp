#include "src/journal_manager/replay/flush_metadata.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"
#include "test/unit-tests/mapper/i_map_flush_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushMetadata, Start_testIfTaskCompletedSuccessfullyWhenAllFlushSuccess)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // Then
    EXPECT_CALL(mapFlush, StoreAll).WillOnce(Return(0));
    EXPECT_CALL(contextManager, FlushContexts).WillOnce(Return(0));

    // When
    int result = flushMetadataTask.Start();
    EXPECT_EQ(result, 0);
}

TEST(FlushMetadata, Start_testIfTaskCompletedWithNegativeValueWhenMapFlushFails)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // Then
    int retCode = -1000;
    EXPECT_CALL(mapFlush, StoreAll).WillOnce(Return(retCode));
    EXPECT_CALL(contextManager, FlushContexts).Times(0);

    // When
    int result = flushMetadataTask.Start();
    EXPECT_EQ(result, retCode);
}

TEST(FlushMetadata, Start_testIfTaskCompletedWithNegativeValueWhenAllocatorContextFlushFails)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // Then
    int retCode = -2000;
    EXPECT_CALL(mapFlush, StoreAll).WillOnce(Return(0));
    EXPECT_CALL(contextManager, FlushContexts).WillOnce(Return(retCode));

    // When
    int result = flushMetadataTask.Start();
    EXPECT_EQ(result, retCode);
}

TEST(FlushMetadata, GetId_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // When
    ReplayTaskId taskId = flushMetadataTask.GetId();

    // Then
    EXPECT_EQ(taskId, ReplayTaskId::FLUSH_METADATA);
}

TEST(FlushMetadata, GetWeight_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // When
    int weight = flushMetadataTask.GetWeight();

    // Then: Executed Successfully without any error
}

TEST(FlushMetadata, GetNumSubTasks__testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockIMapFlush> mapFlush;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockReplayProgressReporter> reporter;

    FlushMetadata flushMetadataTask(&mapFlush, &contextManager, &reporter);

    // When
    int subTasks = flushMetadataTask.GetNumSubTasks();

    // Then: Executed Successfully without any error
}

} // namespace pos
