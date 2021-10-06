#include "src/journal_manager/log/volume_deleted_log_handler.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(VolumeDeletedLogEntry, VolumeDeletedLogEntry_testIfConstructedSuccessfully)
{
    VolumeDeletedLogEntry logHandler(0, 10);
    VolumeDeletedLogEntry* logHandlerInHeap = new VolumeDeletedLogEntry(0, 10);
    delete logHandlerInHeap;

    VolumeDeletedLog log;
    log.volId = 2;
    log.allocatorContextVersion = 32;

    VolumeDeletedLogEntry logHandlerWithLog(log);
    VolumeDeletedLogEntry* logHandlerWithLogInHeap = new VolumeDeletedLogEntry(log);
    delete logHandlerWithLogInHeap;
}

TEST(VolumeDeletedLogEntry, GetType_testIfCorrectTypeIsReturned)
{
    VolumeDeletedLogEntry log(0, 0);
    EXPECT_EQ(log.GetType(), LogType::VOLUME_DELETED);
}

TEST(VolumeDeletedLogEntry, GetSize_testIfCorrectSizeIsReturned)
{
    VolumeDeletedLogEntry log(0, 0);
    EXPECT_EQ(log.GetSize(), sizeof(VolumeDeletedLog));
}

TEST(VolumeDeletedLogEntry, GetData_testIfCorrectDataIsReturned)
{
    VolumeDeletedLog log;
    log.volId = 2;
    log.allocatorContextVersion = 32;

    VolumeDeletedLogEntry logHandler(log);

    VolumeDeletedLog actual = *reinterpret_cast<VolumeDeletedLog*>(logHandler.GetData());
    EXPECT_EQ(actual.volId, log.volId);
    EXPECT_EQ(actual.allocatorContextVersion, log.allocatorContextVersion);
}

TEST(VolumeDeletedLogEntry, GetVsid_testIfUnmapIsReturned)
{
    VolumeDeletedLogEntry log(0, 0);
    EXPECT_EQ(log.GetVsid(), UNMAP_STRIPE);
}

TEST(VolumeDeletedLogEntry, GetSeqNum_testIfCorrectSeqNumIsReturned)
{
    VolumeDeletedLog log;
    log.volId = 2;
    log.allocatorContextVersion = 32;
    log.seqNum = 12;

    VolumeDeletedLogEntry logHandler(log);
    EXPECT_EQ(logHandler.GetSeqNum(), 12);
}

TEST(VolumeDeletedLogEntry, SetSeqNum_testIfSequenceNumberIsUpdated)
{
    VolumeDeletedLog log;
    log.volId = 2;
    log.allocatorContextVersion = 32;

    VolumeDeletedLogEntry logHandler(log);

    logHandler.SetSeqNum(321);
    EXPECT_EQ(logHandler.GetSeqNum(), 321);
}

} // namespace pos
