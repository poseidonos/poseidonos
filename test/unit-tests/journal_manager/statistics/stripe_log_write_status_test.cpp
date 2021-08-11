#include "src/journal_manager/statistics/stripe_log_write_status.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(StripeLogWriteStatus, BlockLogFound_testIfStatusUpdatedWhenBlockLogFound)
{
    // Given
    StripeId vsid = 0U;
    int volId = 5;
    int wbIndex = volId;
    BlkAddr startRba = 200;
    uint32_t numBlks = 1;
    VirtualBlkAddr startVsa{
        .stripeId = vsid,
        .offset = 0UL};
    StripeAddr wbAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 100};

    BlockWriteDoneLog blockLog;
    blockLog.volId = volId;
    blockLog.startRba = startRba;
    blockLog.numBlks = numBlks;
    blockLog.startVsa = startVsa;
    blockLog.wbIndex = wbIndex;
    blockLog.writeBufferStripeAddress = wbAddr;

    StripeLogWriteStatus status(vsid);

    // When
    status.BlockLogFound(blockLog);

    // Then
    BlkOffset startOffset = startVsa.offset;
    EXPECT_EQ(status.GetBlockOffsetRange(), std::make_pair(startOffset, startOffset + numBlks - 1));
    EXPECT_EQ(status.GetRbaRange(), std::make_pair(startRba, startRba));
    EXPECT_EQ(status.GetNumFoundBlocks(), numBlks);
    EXPECT_EQ(status.GetFinalStripeAddr(), wbAddr);
    EXPECT_EQ(status.IsFlushed(), false);
    EXPECT_EQ(status.GetVolumeId(), volId);
    EXPECT_EQ(status.GetWbLsid(), wbAddr.stripeId);
    EXPECT_EQ(status.GetLastOffset(), startOffset + numBlks - 1);
    EXPECT_EQ(status.GetWbIndex(), wbIndex);

    status.Print();
}

TEST(StripeLogWriteStatus, BlockLogFound_testIfStatusUpdatedWhenBlockLogsFound)
{
    // Given
    StripeId vsid = 0U;
    int volId = 5;
    int wbIndex = volId;
    BlkAddr startRba = 200;
    StripeAddr wbAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 100U};

    StripeLogWriteStatus status(vsid);

    // When
    int numBlks = 10;
    for (int offset = 0; offset < numBlks; offset++)
    {
        VirtualBlkAddr startVsa{
            .stripeId = vsid,
            .offset = (BlkOffset)offset};
        BlockWriteDoneLog blockLog;
        blockLog.volId = volId;
        blockLog.startRba = startRba + offset;
        blockLog.numBlks = 1;
        blockLog.startVsa = startVsa;
        blockLog.wbIndex = wbIndex;
        blockLog.writeBufferStripeAddress = wbAddr;

        status.BlockLogFound(blockLog);
    }

    // Then
    BlkOffset startOffset = 0;
    EXPECT_EQ(status.GetBlockOffsetRange(), std::make_pair(startOffset, startOffset + numBlks - 1));
    EXPECT_EQ(status.GetRbaRange(), std::make_pair(startRba, startRba + numBlks - 1));
    EXPECT_EQ(status.GetNumFoundBlocks(), numBlks);
    EXPECT_EQ(status.GetFinalStripeAddr(), wbAddr);
    EXPECT_EQ(status.IsFlushed(), false);
    EXPECT_EQ(status.GetVolumeId(), volId);
    EXPECT_EQ(status.GetWbLsid(), wbAddr.stripeId);
    EXPECT_EQ(status.GetLastOffset(), startOffset + numBlks - 1);
    EXPECT_EQ(status.GetWbIndex(), wbIndex);

    status.Print();
}

TEST(StripeLogWriteStatus, StripeLogFound_testIfStatusUpdatedWhenStripeLogFound)
{
    // Given
    StripeId vsid = 100U;
    StripeId wbLsid = 1U;
    StripeAddr oldAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = wbLsid};

    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = vsid};

    StripeMapUpdatedLog stripeMapLog;
    stripeMapLog.vsid = vsid;
    stripeMapLog.oldMap = oldAddr;
    stripeMapLog.newMap = newAddr;

    StripeLogWriteStatus status(vsid);

    // When
    status.StripeLogFound(stripeMapLog);

    // Then
    BlkOffset startOffset = 0;
    EXPECT_EQ(status.IsFlushed(), true);
    EXPECT_EQ(status.GetWbLsid(), wbLsid);
    EXPECT_EQ(status.GetUserLsid(), vsid);
    EXPECT_EQ(status.GetLastOffset(), UINT64_MAX);
    EXPECT_EQ(status.GetFinalStripeAddr(), stripeMapLog.newMap);

    status.Print();
}

TEST(StripeLogWriteStatus, StripeLogFound_testIfStatusUpdatedWhenBlockLogsAndStripeLogFound)
{
    // Given
    int volId = 5;
    StripeId vsid = 100U;
    StripeId wbLsid = 1U;
    StripeAddr wbAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = wbLsid};
    StripeAddr userAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = vsid};

    int wbIndex = volId;
    BlkAddr startRba = 200;
    int numBlks = 10;

    StripeLogWriteStatus status(vsid);

    // When
    for (int offset = 0; offset < numBlks; offset++)
    {
        VirtualBlkAddr startVsa{
            .stripeId = vsid,
            .offset = (BlkOffset)offset};
        BlockWriteDoneLog blockLog;
        blockLog.volId = volId;
        blockLog.startRba = startRba + offset;
        blockLog.numBlks = 1;
        blockLog.startVsa = startVsa;
        blockLog.wbIndex = wbIndex;
        blockLog.writeBufferStripeAddress = wbAddr;

        status.BlockLogFound(blockLog);
    }

    StripeMapUpdatedLog stripeMapLog;
    stripeMapLog.vsid = vsid;
    stripeMapLog.oldMap = wbAddr;
    stripeMapLog.newMap = userAddr;

    // When
    status.StripeLogFound(stripeMapLog);

    // Then
    EXPECT_EQ(status.IsFlushed(), true);
    EXPECT_EQ(status.GetWbLsid(), wbAddr.stripeId);
    EXPECT_EQ(status.GetUserLsid(), vsid);
    EXPECT_EQ(status.GetLastOffset(), UINT64_MAX);
    EXPECT_EQ(status.GetFinalStripeAddr(), userAddr);

    BlkOffset startOffset = 0;
    EXPECT_EQ(status.GetBlockOffsetRange(), std::make_pair(startOffset, startOffset + numBlks - 1));
    EXPECT_EQ(status.GetRbaRange(), std::make_pair(startRba, startRba + numBlks - 1));
    EXPECT_EQ(status.GetNumFoundBlocks(), numBlks);
    EXPECT_EQ(status.GetVolumeId(), volId);
    EXPECT_EQ(status.GetWbIndex(), wbIndex);

    status.Print();
}

TEST(StripeLogWriteStatus, GcBlockLogFound_testIfStatusUpdatedWhenGcBlocksAndGcStripeFound)
{
    // Given
    int volId = 5;
    StripeId vsid = 100U;
    StripeId wbLsid = 1U;
    StripeId userLsid = 100U;
    int numBlockMaps = 10;
    BlkAddr startRba = 200;

    StripeAddr userAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = vsid};

    GcBlockMapUpdate gcBlockLog[numBlockMaps];
    for (uint64_t offset = 0; offset < numBlockMaps; offset++)
    {
        gcBlockLog[offset].rba = startRba + offset;
        gcBlockLog[offset].vsa = {
            .stripeId = vsid,
            .offset = offset};
    }

    GcStripeFlushedLog gcStripeLog;
    gcStripeLog.volId = volId;
    gcStripeLog.vsid = vsid;
    gcStripeLog.wbLsid = wbLsid;
    gcStripeLog.userLsid = userLsid;
    gcStripeLog.totalNumBlockMaps = numBlockMaps;

    StripeLogWriteStatus status(vsid);

    // When
    status.GcBlockLogFound(gcBlockLog, numBlockMaps);
    status.GcStripeLogFound(gcStripeLog);

    // Then
    EXPECT_EQ(status.IsFlushed(), true);
    EXPECT_EQ(status.GetUserLsid(), vsid);
    EXPECT_EQ(status.GetFinalStripeAddr(), userAddr);

    BlkOffset startOffset = 0;
    EXPECT_EQ(status.GetBlockOffsetRange(), std::make_pair(startOffset, startOffset + numBlockMaps - 1));
    EXPECT_EQ(status.GetRbaRange(), std::make_pair(startRba, startRba + numBlockMaps - 1));
    EXPECT_EQ(status.GetNumFoundBlocks(), numBlockMaps);
    EXPECT_EQ(status.GetVolumeId(), volId);
}

} // namespace pos
