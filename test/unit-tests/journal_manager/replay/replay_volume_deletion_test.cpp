#include <gtest/gtest.h>
#include "src/journal_manager/replay/replay_volume_deletion.h"

#include "test/unit-tests/journal_manager/replay/replay_progress_reporter_mock.h"
#include "test/unit-tests/journal_manager/replay/log_delete_checker_mock.h"

#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using ::testing::SetArgReferee;

namespace pos
{
TEST(ReplayVolumeDeletion, Start_testIfVolumeNotDeletedWhenVersionIsUpdatedToNewest)
{
    // Given
    NiceMock<MockLogDeleteChecker> logDeleteChecker;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockReplayProgressReporter> progressReporter;

    uint64_t prevContextVersion = 2;
    uint64_t storedContextVersion = 3;
    ON_CALL(contextManager, GetStoredContextVersion(SEGMENT_CTX)).
        WillByDefault(Return(storedContextVersion));

    int volId = 1;
    std::string volname = "volume";

    std::vector<DeletedVolume> deletedVolumes;
    deletedVolumes.push_back(DeletedVolume{volId, 0, prevContextVersion});
    ON_CALL(logDeleteChecker, GetDeletedVolumes).WillByDefault(Return(deletedVolumes));

    ON_CALL(volumeManager, GetVolumeStatus(volId)).WillByDefault(Return(0));
    EXPECT_CALL(volumeManager, VolumeName(volId, _)).WillRepeatedly(SetArgReferee<1>(volname));

    // Then
    EXPECT_CALL(volumeManager, Delete(volname)).WillOnce(Return(0));

    // When
    ReplayVolumeDeletion replayer(&logDeleteChecker, &contextManager, &volumeManager, &progressReporter);
    replayer.Start();
}

TEST(ReplayVolumeDeletion, Start_testIfVolumeNotDeletedWhenVersionIsNotUpdated)
{
    // Given
    NiceMock<MockLogDeleteChecker> logDeleteChecker;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockReplayProgressReporter> progressReporter;

    uint64_t prevContextVersion = 2;
    uint64_t storedContextVersion = 2;
    ON_CALL(contextManager, GetStoredContextVersion(SEGMENT_CTX)).
        WillByDefault(Return(storedContextVersion));

    int volId = 1;
    std::string volname = "volume";

    std::vector<DeletedVolume> deletedVolumes;
    deletedVolumes.push_back(DeletedVolume{volId, 0, prevContextVersion});
    ON_CALL(logDeleteChecker, GetDeletedVolumes).WillByDefault(Return(deletedVolumes));

    // Then
    EXPECT_CALL(volumeManager, Delete(volname)).Times(0);

    // When
    ReplayVolumeDeletion replayer(&logDeleteChecker, &contextManager, &volumeManager, &progressReporter);
    replayer.Start();
}

TEST(ReplayVolumeDeletion, Start_testIfVolumeNotDeletedWhenVolumeDoesNotExist)
{
    // Given
    NiceMock<MockLogDeleteChecker> logDeleteChecker;
    NiceMock<MockIContextManager> contextManager;
    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockReplayProgressReporter> progressReporter;

    uint64_t prevContextVersion = 2;
    uint64_t storedContextVersion = 2;
    ON_CALL(contextManager, GetStoredContextVersion(SEGMENT_CTX)).
        WillByDefault(Return(storedContextVersion));

    int volId = 1;
    std::string volname = "volume";

    std::vector<DeletedVolume> deletedVolumes;
    deletedVolumes.push_back(DeletedVolume{volId, 0, prevContextVersion});
    ON_CALL(logDeleteChecker, GetDeletedVolumes).WillByDefault(Return(deletedVolumes));

    ON_CALL(volumeManager, GetVolumeStatus(volId)).WillByDefault(Return((int)(POS_EVENT_ID::VOL_NOT_FOUND)));

    // Then
    EXPECT_CALL(volumeManager, Delete(volname)).Times(0);

    // When
    ReplayVolumeDeletion replayer(&logDeleteChecker, &contextManager, &volumeManager, &progressReporter);
    replayer.Start();
}
}  // namespace pos
