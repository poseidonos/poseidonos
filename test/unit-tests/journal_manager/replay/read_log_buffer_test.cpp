#include "src/journal_manager/replay/read_log_buffer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log/log_buffer_parser_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_log_list_mock.h"
#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(ReadLogBuffer, Start_testIfReadSuccessWhenLogIsReadFromAllLogGroups)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockJournalLogBuffer> logBuffer;
    NiceMock<MockReplayLogList> replayLogList;
    NiceMock<MockReplayProgressReporter> progressReporter;
    NiceMock<MockLogBufferParser>* parser = new NiceMock<MockLogBufferParser>;

    ReadLogBuffer readLogBufferTask(&config, &logBuffer, replayLogList, &progressReporter, parser);

    // When
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));
    ON_CALL(config, GetLogGroupSize).WillByDefault(Return(16 * 1024));

    EXPECT_CALL(logBuffer, ReadLogBuffer).WillRepeatedly(Return(0));
    EXPECT_CALL(*parser, GetLogs).WillRepeatedly(Return(0));

    int result = readLogBufferTask.Start();

    // Then
    EXPECT_EQ(result, 0);
}

TEST(ReadLogBuffer, Start_testIfReadSuccessWhenLogBufferReadFails)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockJournalLogBuffer> logBuffer;
    NiceMock<MockReplayLogList> replayLogList;
    NiceMock<MockReplayProgressReporter> progressReporter;
    NiceMock<MockLogBufferParser>* parser = new NiceMock<MockLogBufferParser>;

    ReadLogBuffer readLogBufferTask(&config, &logBuffer, replayLogList, &progressReporter, parser);

    // When
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));
    ON_CALL(config, GetLogGroupSize).WillByDefault(Return(16 * 1024));

    int retCode = -1000;
    EXPECT_CALL(logBuffer, ReadLogBuffer).WillOnce(Return(0)).WillOnce(Return(retCode));
    EXPECT_CALL(*parser, GetLogs).WillRepeatedly(Return(0));

    int result = readLogBufferTask.Start();

    // Then
    EXPECT_EQ(result, retCode);
}

TEST(ReadLogBuffer, Start_testIfReadSuccessWhenLogBufferParseFails)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockJournalLogBuffer> logBuffer;
    NiceMock<MockReplayLogList> replayLogList;
    NiceMock<MockReplayProgressReporter> progressReporter;
    NiceMock<MockLogBufferParser>* parser = new NiceMock<MockLogBufferParser>;

    ReadLogBuffer readLogBufferTask(&config, &logBuffer, replayLogList, &progressReporter, parser);

    // When
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));
    ON_CALL(config, GetLogGroupSize).WillByDefault(Return(16 * 1024));

    int retCode = -3000;
    EXPECT_CALL(logBuffer, ReadLogBuffer).WillRepeatedly(Return(0));
    EXPECT_CALL(*parser, GetLogs).WillOnce(Return(0)).WillOnce(Return(retCode));

    int result = readLogBufferTask.Start();

    // Then
    EXPECT_EQ(result, retCode);
}

TEST(ReadLogBuffer, GetId_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockReplayLogList> replayLogList;
    ReadLogBuffer readLogBufferTask(nullptr, nullptr, replayLogList, nullptr, nullptr);

    // When
    ReplayTaskId taskId = readLogBufferTask.GetId();

    // Then
    EXPECT_EQ(taskId, ReplayTaskId::READ_LOG_BUFFER);
}

TEST(ReadLogBuffer, GetWeight_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockReplayLogList> replayLogList;
    ReadLogBuffer readLogBufferTask(nullptr, nullptr, replayLogList, nullptr, nullptr);

    // When
    int weight = readLogBufferTask.GetWeight();

    // Then: Executed Successfully without any error
}

TEST(ReadLogBuffer, GetNumSubTasks_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockReplayLogList> replayLogList;
    ReadLogBuffer readLogBufferTask(&config, nullptr, replayLogList, nullptr, nullptr);

    // When
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(2));

    int numSubTasks = readLogBufferTask.GetNumSubTasks();

    // Then: numSubTasks should be same with the numLogGroups
    EXPECT_EQ(numSubTasks, 2);
}

} // namespace pos
