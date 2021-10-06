#include "src/journal_manager/log/block_write_done_log_handler.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(BlockWriteDoneLogHandler, BlockWriteDoneLogHandler_testIfConstructedSuccessfully)
{
    int volId = 0;
    BlkAddr startRba = 100;
    uint32_t numBlks = 10;
    VirtualBlkAddr startVsa = {
        .stripeId = 13,
        .offset = 1};
    int wbIndex = 0;
    StripeAddr stripeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};

    BlockWriteDoneLogHandler logHandler(volId, startRba, numBlks, startVsa, wbIndex, stripeAddr);
    BlockWriteDoneLogHandler* logHandlerInHeap = new BlockWriteDoneLogHandler(volId, startRba, numBlks, startVsa, wbIndex, stripeAddr);

    BlockWriteDoneLog log;
    log.type = LogType::BLOCK_WRITE_DONE;
    log.seqNum = 0;
    log.volId = volId;
    log.startRba = startRba;
    log.numBlks = numBlks;
    log.startVsa = startVsa;
    log.wbIndex = wbIndex;
    log.writeBufferStripeAddress = stripeAddr;

    BlockWriteDoneLogHandler logHandlerWithLog(log);
    BlockWriteDoneLogHandler* logHandlerWithLogInHeap = new BlockWriteDoneLogHandler(log);

    delete logHandlerInHeap;
    delete logHandlerWithLogInHeap;
}

TEST(BlockWriteDoneLogHandler, GetType_testIfCorrectTypeIsReturned)
{
    BlockWriteDoneLogHandler logHandler(0, 0, 1, UNMAP_VSA, 0, StripeAddr{IN_WRITE_BUFFER_AREA, UNMAP_STRIPE});
    EXPECT_EQ(logHandler.GetType(), LogType::BLOCK_WRITE_DONE);
}

TEST(BlockWriteDoneLogHandler, GetSize_testIfCorrectSizeIsReturned)
{
    BlockWriteDoneLogHandler logHandler(0, 0, 1, UNMAP_VSA, 0, StripeAddr{IN_WRITE_BUFFER_AREA, UNMAP_STRIPE});
    EXPECT_EQ(logHandler.GetSize(), sizeof(BlockWriteDoneLog));
}

TEST(BlockWriteDoneLogHandler, GetData_testIfCorrectDataIsReturned)
{
    int volId = 0;
    BlkAddr startRba = 100;
    uint32_t numBlks = 10;
    VirtualBlkAddr startVsa = {
        .stripeId = 13,
        .offset = 1};
    int wbIndex = 0;
    StripeAddr stripeAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};

    BlockWriteDoneLogHandler logHandler(volId, startRba, numBlks, startVsa, wbIndex, stripeAddr);

    BlockWriteDoneLog actual = *reinterpret_cast<BlockWriteDoneLog*>(logHandler.GetData());
    EXPECT_EQ(actual.type, LogType::BLOCK_WRITE_DONE);
    EXPECT_EQ(actual.volId, volId);
    EXPECT_EQ(actual.startRba, startRba);
    EXPECT_EQ(actual.numBlks, numBlks);
    EXPECT_EQ(actual.wbIndex, wbIndex);
    EXPECT_EQ(actual.writeBufferStripeAddress, stripeAddr);
}

TEST(BlockWriteDoneLogHandler, GetVsid_testIfCorrectVsidIsReturned)
{
    StripeId stripeId = 251;
    BlockWriteDoneLogHandler logHandler(0, 0, 1,
        VirtualBlkAddr{stripeId, IN_WRITE_BUFFER_AREA},
        0,
        StripeAddr{IN_WRITE_BUFFER_AREA, UNMAP_STRIPE});

    EXPECT_EQ(logHandler.GetVsid(), stripeId);
}

TEST(BlockWriteDoneLogHandler, GetSeqNum_testIfCorrectSeqNumIsReturned)
{
    BlockWriteDoneLog log;
    log.type = LogType::BLOCK_WRITE_DONE;
    log.seqNum = 126;

    BlockWriteDoneLogHandler logHandler(log);
    EXPECT_EQ(logHandler.GetSeqNum(), log.seqNum);
}

TEST(BlockWriteDoneLogHandler, SetSeqNum_testIfSequenceNumberIsUpdated)
{
    BlockWriteDoneLogHandler logHandler(0, 0, 1,
        VirtualBlkAddr{UNMAP_STRIPE, IN_WRITE_BUFFER_AREA},
        0,
        StripeAddr{IN_WRITE_BUFFER_AREA, UNMAP_STRIPE});

    logHandler.SetSeqNum(14);
    EXPECT_EQ(logHandler.GetSeqNum(), 14);
}

} // namespace pos
