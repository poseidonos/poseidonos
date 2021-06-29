#include "src/journal_manager/replay/log_delete_checker.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log/log_handler_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
char*
CreateVolumeDeletedLog(int volId, uint64_t ver)
{
    char* data = (char*)malloc(sizeof(VolumeDeletedLog));
    VolumeDeletedLog* logPtr = reinterpret_cast<VolumeDeletedLog*>(data);
    logPtr->mark = LOG_VALID_MARK;
    logPtr->volId = volId;
    logPtr->allocatorContextVersion = ver;

    return data;
}

void
FreeVolumeDeletedLog(char* buf)
{
    free(buf);
}

TEST(LogDeleteChecker, Update_testIfReplayLogListIsChangedToDeletedVolumeList)
{
    // Given
    std::vector<ReplayLog> replayLogs;
    std::vector<char*> buffers;
    for (int count = 0; count < 10; count++)
    {
        int volId = count;
        uint64_t ver = count;
        uint64_t time = count;

        MockLogHandlerInterface* log = new NiceMock<MockLogHandlerInterface>;
        char* buf = CreateVolumeDeletedLog(volId, ver);
        ON_CALL(*log, GetData).WillByDefault(Return(buf));

        replayLogs.push_back(ReplayLog{time, log});
        buffers.push_back(buf);
    }

    // When
    LogDeleteChecker checker;
    checker.Update(replayLogs);

    // Then
    std::vector<DeletedVolume> deletedVolumeInfos = checker.GetDeletedVolumes();
    for (int count = 0; count < 10; count++)
    {
        int volId = count;
        uint64_t ver = count;
        uint64_t time = count;

        EXPECT_EQ(deletedVolumeInfos[count].prevSegInfoVersion, ver);
        EXPECT_EQ(deletedVolumeInfos[count].volumeId, volId);
        EXPECT_EQ(deletedVolumeInfos[count].time, time);
    }

    for (int count = 0; count < 10; count++)
    {
        delete replayLogs[count].log;
        FreeVolumeDeletedLog(buffers[count]);
    }
    replayLogs.clear();
    buffers.clear();
}

TEST(LogDeleteChecker, ReplayedUntil_testIfReplayedVolumesAreErased)
{
    // Given
    int volumeId = 2;
    std::vector<DeletedVolume> volumeList;
    for (uint64_t time = 0; time < 5; time++)
    {
        volumeList.push_back(DeletedVolume{volumeId, time, 0});
    }

    // When
    LogDeleteChecker checker(volumeList);
    checker.ReplayedUntil(3, volumeId);

    // Then
    std::vector<DeletedVolume> actual = checker.GetDeletedVolumes();
    EXPECT_EQ(actual.size(), 2);
    EXPECT_EQ(actual[0].time, 3);
    EXPECT_EQ(actual[1].time, 4);
}

TEST(LogDeleteChecker, ReplayedUntil_testIfVolumesAreNotErasedWhenOtherVolumeMetaIsReplayed)
{
    // Given
    int volumeId = 2;
    std::vector<DeletedVolume> volumeList;
    for (uint64_t time = 0; time < 5; time++)
    {
        volumeList.push_back(DeletedVolume{volumeId, time, 0});
    }

    // When
    LogDeleteChecker checker(volumeList);
    checker.ReplayedUntil(3, 1);

    // Then
    std::vector<DeletedVolume> actual = checker.GetDeletedVolumes();
    EXPECT_EQ(volumeList, actual);
}

TEST(LogDeleteChecker, IsDeleted_testIfReturnCorrectly)
{
    // Given
    std::vector<DeletedVolume> volumeList;
    for (int volId = 0; volId < 5; volId++)
    {
        volumeList.push_back(DeletedVolume{volId, 0, 0});
    }
    LogDeleteChecker checker(volumeList);

    // When

    // Then
    EXPECT_EQ(checker.IsDeleted(0), true);
    EXPECT_EQ(checker.IsDeleted(10), false);
}

} // namespace pos
