#include "src/journal_manager/status/journal_status_provider.h"

#include <gtest/gtest.h>

#include <string>

#include "src/journal_manager/checkpoint/log_group_releaser.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/status/i_checkpoint_status_mock.h"
#include "test/unit-tests/journal_manager/status/i_log_buffer_status_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(JournalStatusProvider, Init_testIfExecutedSuccess)
{
    // Given
    JournalStatusProvider provider;

    // When
    provider.Init(nullptr, nullptr, nullptr);

    // Then
}

TEST(JournalStatusProvider, GetJournalStatus_testIfGetConfigElementSuccess)
{
    // Given
    JournalStatusProvider provider;
    NiceMock<MockICheckpointStatus> checkpointStatus;
    NiceMock<MockILogBufferStatus> logBufferStatus;
    NiceMock<MockJournalConfiguration> config;
    provider.Init(&logBufferStatus, &config, &checkpointStatus);

    // When
    int numLogGroup = 2;
    int logBufferSize = 1024;
    int sequenceNumber = 0;
    int flushingLogGroupId = -1;
    std::list<int> fullLogGroups;
    fullLogGroups.push_back(0);
    fullLogGroups.push_back(1);

    ON_CALL(config, GetLogBufferSize).WillByDefault(Return(logBufferSize));
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroup));

    ON_CALL(logBufferStatus, GetSequenceNumber).WillByDefault(Return(sequenceNumber));
    ON_CALL(logBufferStatus, GetBufferStatus).WillByDefault(Return(LogGroupStatus::FULL));

    ON_CALL(checkpointStatus, GetFlushingLogGroupId).WillByDefault(Return(flushingLogGroupId));
    ON_CALL(checkpointStatus, GetFullLogGroups).WillByDefault(Return(fullLogGroups));
    ON_CALL(checkpointStatus, GetStatus).WillByDefault(Return(CheckpointStatus::INIT));

    ElementList actual = provider.GetJournalStatus();

    // Then
    // This expect value should be updated according to the changes of expect values above
    std::string expect = "\"config\":{\"numLogGroups\":" + std::to_string(numLogGroup) +
        "}\"logBufferStatus\":{\"logBufferSizeInBytes\":" + std::to_string(logBufferSize) +
        ",\"logGroups\":[{\"seqNum\":" + std::to_string(sequenceNumber) +
        ",\"status\":\"FULL\"},{\"seqNum\":" + std::to_string(sequenceNumber) +
        ",\"status\":\"FULL\"}]}\"checkpointStatus\":{\"flushingLogGroupId\":" + to_string(flushingLogGroupId) +
        ",\"status\":\"INIT\",\"fullLogGroups\":[{\"ID\":0},{\"ID\":1}]}";

    std::string actualString;
    for (auto it : actual)
    {
        actualString += it.ToJson();
    }

    EXPECT_EQ(expect, actualString);
}

} // namespace pos
