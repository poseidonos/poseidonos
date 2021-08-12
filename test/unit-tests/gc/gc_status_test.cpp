#include "src/gc/gc_status.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(CopyInfo, CopyInfo_CreateWithVictimSegmentId)
{
    // given segment Id
    uint32_t testVictimSegment = 0;
    // when create copy info
    CopyInfo copyInfo(testVictimSegment);

    // then return segment id and start time
    EXPECT_TRUE(copyInfo.GetSegmentId() == testVictimSegment);
    struct timeval startTime = copyInfo.GetStartTime();
    EXPECT_TRUE(startTime.tv_sec != 0);
}

TEST(CopyInfo, SetInfo_SetInfoWithInvalidAndCopiedBlockCount)
{
    // given segment Id and create copy info
    uint32_t testVictimSegment = 10;
    CopyInfo copyInfo(testVictimSegment);
    uint32_t invalidCnt = 110;
    uint32_t copyDoneCnt = 33;

    // when SetInfo with invalid, copied block count
    copyInfo.SetInfo(invalidCnt, copyDoneCnt);

    // then return segment id, invalid & copied block count, start & end time
    EXPECT_TRUE(copyInfo.GetSegmentId() == testVictimSegment);
    EXPECT_TRUE(copyInfo.GetInvalidBlkCnt() == invalidCnt);
    EXPECT_TRUE(copyInfo.GetCopiedBlkCnt() == copyDoneCnt);
    struct timeval startTime = copyInfo.GetStartTime();
    struct timeval endTime = copyInfo.GetEndTime();
    EXPECT_TRUE(startTime.tv_sec != 0);
    EXPECT_TRUE(endTime.tv_sec != 0);
}

} // namespace pos

namespace pos
{
TEST(GcStatus, SetCopyInfo_SetCopyInfoStarted)
{
    // given test argument
    uint32_t testVictimSegment = 0;
    uint32_t invalidCnt = 0;
    uint32_t copyDoneCnt = 0;
    GcStatus gcStatus;
    bool started = false;

    // when set starting copy info
    gcStatus.SetCopyInfo(started, testVictimSegment, invalidCnt, copyDoneCnt);

    // then return empty copy info list
    std::queue<CopyInfo> copyInfoList = gcStatus.GetCopyInfoList();
    EXPECT_TRUE(copyInfoList.empty() == true);
    bool gcRunning = gcStatus.GetGcRunning();
    EXPECT_TRUE(gcRunning == true);
    struct timeval startTime = gcStatus.GetStartTime();
    struct timeval endTime = gcStatus.GetEndTime();
    EXPECT_TRUE(startTime.tv_sec != 0);
    EXPECT_TRUE(endTime.tv_sec == 0);
}

TEST(GcStatus, SetCopyInfo_SetCopyInfoStartedAndSetCopyInfoEnded)
{
    // given test argument
    uint32_t testVictimSegment = 120;
    uint32_t invalidCnt = 630;
    uint32_t copyDoneCnt = 550;
    GcStatus gcStatus;
    bool started = false;

    // when set starting copy info
    gcStatus.SetCopyInfo(started, testVictimSegment, invalidCnt, copyDoneCnt);

    // then return empty copy info list
    std::queue<CopyInfo> copyInfoList = gcStatus.GetCopyInfoList();
    EXPECT_TRUE(copyInfoList.empty() == true);
    bool gcRunning = gcStatus.GetGcRunning();
    EXPECT_TRUE(gcRunning == true);
    struct timeval startTime = gcStatus.GetStartTime();
    struct timeval endTime = gcStatus.GetEndTime();
    EXPECT_TRUE(startTime.tv_sec != 0);
    EXPECT_TRUE(endTime.tv_sec == 0);

    // when set ending copy info
    started = true;
    gcStatus.SetCopyInfo(started, testVictimSegment, invalidCnt, copyDoneCnt);

    // then return copy info with test argument
    copyInfoList = gcStatus.GetCopyInfoList();
    EXPECT_TRUE(copyInfoList.empty() != true);
    gcRunning = gcStatus.GetGcRunning();
    EXPECT_TRUE(gcRunning == false);
    CopyInfo copyInfo = copyInfoList.front();
    EXPECT_TRUE(copyInfo.GetSegmentId() == testVictimSegment);
    EXPECT_TRUE(copyInfo.GetInvalidBlkCnt() == invalidCnt);
    EXPECT_TRUE(copyInfo.GetCopiedBlkCnt() == copyDoneCnt);
}

} // namespace pos
