#include "src/journal_manager/replay/replay_handler.h"

#include <gtest/gtest.h>

#include <vector>

#include "src/journal_manager/replay/log_delete_checker.h"
#include "test/unit-tests/allocator/i_context_replayer_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/replay/log_delete_checker_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"

using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReplayHandler, ReplayHandler_)
{
}

TEST(ReplayHandler, Init_testIfAllTasksRegisteredSuccessfully)
{
    // Given
    NiceMock<MockIStateControl> iStateControl;
    NiceMock<MockLogDeleteChecker>* logDeleteChecker = new NiceMock<MockLogDeleteChecker>;
    NiceMock<MockReplayProgressReporter>* replayProgressReporter = new NiceMock<MockReplayProgressReporter>;
    NiceMock<MockJournalConfiguration> journalConfiguration;
    NiceMock<MockIContextReplayer> iContextReplayer;

    int numLogGroups = 2;
    EXPECT_CALL(iStateControl, Subscribe);
    ReplayHandler replayHandler(&iStateControl, logDeleteChecker, replayProgressReporter);

    EXPECT_CALL(journalConfiguration, GetNumLogGroups).WillOnce(Return(numLogGroups));
    EXPECT_CALL(iContextReplayer, GetAllActiveStripeTail);
    {
        // Then: All replay tasks will be registered in this sequence
        InSequence s;
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::READ_LOG_BUFFER, 40));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::FILTER_LOGS, 10));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::REPLAY_LOGS, 30));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::REPLAY_VOLUME_DELETION, 10));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::FLUSH_METADATA, 20));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::RESET_LOG_BUFFER, 10));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::LOAD_REPLAYED_SEGMENT_CONTEXT, 10));
        EXPECT_CALL(*replayProgressReporter, RegisterTask(ReplayTaskId::FLUSH_PENDING_STRIPES, 10));
    }

    // When
    replayHandler.Init(&journalConfiguration, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &iContextReplayer, nullptr, nullptr, nullptr);
}

TEST(ReplayHandler, InitDispose_testInitAndDisposeRepeat)
{
    // Given
    NiceMock<MockIStateControl> iStateControl;
    NiceMock<MockLogDeleteChecker>* logDeleteChecker = new NiceMock<MockLogDeleteChecker>;
    NiceMock<MockReplayProgressReporter>* replayProgressReporter = new NiceMock<MockReplayProgressReporter>;
    NiceMock<MockJournalConfiguration> journalConfiguration;
    NiceMock<MockIContextReplayer> iContextReplayer;

    int numLogGroups = 2;
    EXPECT_CALL(iStateControl, Subscribe);
    ReplayHandler replayHandler(&iStateControl, logDeleteChecker, replayProgressReporter);

    replayHandler.Init(&journalConfiguration, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &iContextReplayer, nullptr, nullptr, nullptr);
    replayHandler.Dispose();

    replayHandler.Init(&journalConfiguration, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &iContextReplayer, nullptr, nullptr, nullptr);
    replayHandler.Dispose();
}

} // namespace pos
