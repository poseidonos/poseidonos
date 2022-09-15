#include "src/journal_manager/checkpoint/checkpoint_meta_flush_completed.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/checkpoint_handler_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(CheckpointMetaFlushCompleted, Execute_testIfExecuteSuccessfully)
{
    // Given: Map flush completed event with mapId
    NiceMock<MockCheckpointHandler> checkpointHandler(0);
    int mapId = 0;
    int logGroupId = 0;
    CheckpointMetaFlushCompleted metaFlushCompleted(&checkpointHandler, mapId, logGroupId);

    // Then: Checkpoint handler should be notified with the mapId
    EXPECT_CALL(checkpointHandler,
        FlushCompleted(mapId, logGroupId))
        .Times(1)
        .WillOnce(Return(0));

    // When
    bool result = metaFlushCompleted.Execute();

    // Then
    EXPECT_EQ(true, result);
}

TEST(CheckpointMetaFlushCompleted, Execute_testIfExecuteFails)
{
    // Given: Map flush completed event with mapId
    NiceMock<MockCheckpointHandler> checkpointHandler(0);
    int mapId = 0;
    int logGroupId = 0;
    CheckpointMetaFlushCompleted metaFlushCompleted(&checkpointHandler, mapId, logGroupId);

    // Then: Checkpoint handler should be notified with the mapId
    EXPECT_CALL(checkpointHandler,
        FlushCompleted(mapId, logGroupId))
        .Times(1)
        .WillOnce(Return(-1));

    // When
    bool result = metaFlushCompleted.Execute();

    // Then
    EXPECT_EQ(false, result);
}
} // namespace pos
