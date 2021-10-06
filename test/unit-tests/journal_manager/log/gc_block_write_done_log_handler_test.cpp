#include "src/journal_manager/log/gc_block_write_done_log_handler.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(GcBlockWriteDoneLogHandler, GcBlockWriteDoneLogHandler_testIfLogDataIsCreatedProperly)
{
    // Given
    int volumeId = 2;
    StripeId vsid = 230;
    GcBlockMapUpdateList blockMapUpdateList;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = vsid,
                .offset = offset}};
        blockMapUpdateList.push_back(update);
    }

    // When
    GcBlockWriteDoneLogHandler handler(volumeId, vsid, blockMapUpdateList);

    // Then
    EXPECT_EQ(handler.GetType(), LogType::GC_BLOCK_WRITE_DONE);
    EXPECT_EQ(handler.GetSize(), sizeof(GcBlockWriteDoneLog) + sizeof(GcBlockMapUpdate) * 128);
    EXPECT_EQ(handler.GetVsid(), vsid);
}

TEST(GcBlockWriteDoneLogHandler, operator_testIfReturnsTrueWhenItsDataIsExactlySame)
{
    // Given
    GcBlockMapUpdateList blockMapUpdateList;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = 230,
                .offset = offset}};
        blockMapUpdateList.push_back(update);
    }

    // When
    GcBlockWriteDoneLogHandler handler1(2, 230, blockMapUpdateList);
    GcBlockWriteDoneLogHandler handler2(2, 230, blockMapUpdateList);

    EXPECT_TRUE(handler1 == handler2);
}

TEST(GcBlockWriteDoneLogHandler, operator_testIfReturnsFalseWhenBlockMapIsDifferent)
{
    GcBlockMapUpdateList blockMapUpdateList1;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = 230,
                .offset = offset}};
        blockMapUpdateList1.push_back(update);
    }
    GcBlockWriteDoneLogHandler handler1(2, 230, blockMapUpdateList1);

    GcBlockMapUpdateList blockMapUpdateList2;
    for (uint64_t offset = 0; offset < 10; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = 230,
                .offset = offset}};
        blockMapUpdateList2.push_back(update);
    }
    GcBlockWriteDoneLogHandler handler2(2, 230, blockMapUpdateList2);

    EXPECT_FALSE(handler1 == handler2);
}

TEST(GcBlockWriteDoneLogHandler, operator_testIfReturnsFalseWhenStripeAddressIsDifferent)
{
    GcBlockMapUpdateList dummyBlockMapUpdateList;

    GcBlockWriteDoneLogHandler handler1(2, 230, dummyBlockMapUpdateList);
    GcBlockWriteDoneLogHandler handler2(5, 230, dummyBlockMapUpdateList);

    EXPECT_FALSE(handler1 == handler2);
}

TEST(GcBlockWriteDoneLogHandler, GetData_testIfDataIsReturnedProperly)
{
    // Given
    int volumeId = 5;
    StripeId vsid = 102;
    GcBlockMapUpdateList blockMapUpdateList;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset,
            .vsa = {
                .stripeId = vsid,
                .offset = offset}};
        blockMapUpdateList.push_back(update);
    }

    // When
    GcBlockWriteDoneLogHandler handler(volumeId, vsid, blockMapUpdateList);
    char* data = handler.GetData();

    // Then
    GcBlockWriteDoneLog* logPtr = reinterpret_cast<GcBlockWriteDoneLog*>(data);
    EXPECT_EQ(logPtr->type, LogType::GC_BLOCK_WRITE_DONE);
    EXPECT_EQ(logPtr->volId, volumeId);
    EXPECT_EQ(logPtr->vsid, vsid);

    GcBlockMapUpdate* mapListPtr = reinterpret_cast<GcBlockMapUpdate*>(data + sizeof(GcBlockWriteDoneLog));
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate expected{.rba = offset, .vsa = {.stripeId = vsid, .offset = offset}};
        GcBlockMapUpdate actual = mapListPtr[offset];

        EXPECT_EQ(expected.rba, actual.rba);
        EXPECT_EQ(expected.vsa, actual.vsa);
    }
}

TEST(GcBlockWriteDoneLogHandler, SetSeqNum_testIfSeqNumIsSetCorrectly)
{
    int volumeId = 2;
    StripeId vsid = 230;
    GcBlockMapUpdateList blockMapUpdateList;
    GcBlockWriteDoneLogHandler handler(volumeId, vsid, blockMapUpdateList);

    handler.SetSeqNum(10234);

    EXPECT_EQ(handler.GetSeqNum(), 10234);
}

TEST(GcBlockWriteDoneLogHandler, GetGcBlockMapWriteDoneLog_testIfLogPtrIsReturned)
{
    // Given
    int volumeId = 10;
    StripeId vsid = 122;
    GcBlockMapUpdateList blockMapUpdateList;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset + 1024,
            .vsa = {
                .stripeId = vsid,
                .offset = offset}};
        blockMapUpdateList.push_back(update);
    }

    // When
    GcBlockWriteDoneLogHandler handler(volumeId, vsid, blockMapUpdateList);
    GcBlockWriteDoneLog* logPtr = handler.GetGcBlockMapWriteDoneLog();

    // Then
    EXPECT_EQ(logPtr->type, LogType::GC_BLOCK_WRITE_DONE);
    EXPECT_EQ(logPtr->volId, volumeId);
    EXPECT_EQ(logPtr->vsid, vsid);
}

TEST(GcBlockWriteDoneLogHandler, GetMapList_testIfMapListIsReturned)
{
    // Given
    int volumeId = 15;
    StripeId vsid = 402;
    GcBlockMapUpdateList blockMapUpdateList;
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate update = {
            .rba = offset + 512,
            .vsa = {
                .stripeId = vsid,
                .offset = offset}};
        blockMapUpdateList.push_back(update);
    }

    // When
    GcBlockWriteDoneLogHandler handler(volumeId, vsid, blockMapUpdateList);
    GcBlockMapUpdate* mapList = handler.GetMapList();

    // Then
    for (uint64_t offset = 0; offset < 128; offset++)
    {
        GcBlockMapUpdate expected = {
            .rba = offset + 512,
            .vsa = {
                .stripeId = vsid,
                .offset = offset}};
        GcBlockMapUpdate actual = mapList[offset];

        EXPECT_EQ(expected.rba, actual.rba);
        EXPECT_EQ(expected.vsa, actual.vsa);
    }
}

} // namespace pos
