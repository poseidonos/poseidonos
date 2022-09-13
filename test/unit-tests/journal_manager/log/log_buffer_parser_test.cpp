#include "src/journal_manager/log/log_buffer_parser.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "src/journal_manager/log/gc_map_update_list.h"
#include "test/unit-tests/journal_manager/log/log_list_mock.h"

using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
MATCHER_P(EqLog, expected, "Equality matcher for log")
{
    return (memcmp(expected, arg->GetData(), arg->GetSize()) == 0);
}

TEST(LogBufferParser, LogBufferParser_testIfConstructedSuccessfully)
{
    LogBufferParser parser;

    LogBufferParser* parserInHeap = new LogBufferParser();
    delete parserInHeap;
}

TEST(LogBufferParser, GetLogs_testIfBlockWriteDoneLogIsParsed)
{
    // Given
    BlockWriteDoneLog log;
    log.type = LogType::BLOCK_WRITE_DONE;
    log.volId = 1;
    log.startRba = 0;
    log.numBlks = 0;
    log.startVsa = {
        .stripeId = 0,
        .offset = 0};
    log.wbIndex = 0;
    log.writeBufferStripeAddress = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};

    uint64_t bufferSize = sizeof(log);
    void* buffer = malloc(bufferSize);
    memcpy(buffer, &log, bufferSize);

    NiceMock<MockLogList> logList;

    // Then
    EXPECT_CALL(logList, AddLog(EqLog(buffer)));

    // When
    LogBufferParser parser;
    parser.GetLogs(buffer, bufferSize, logList);

    free(buffer);
}

TEST(LogBufferParser, GetLogs_testIfStripeMapUpdatedLogIsParsed)
{
    // Given
    StripeMapUpdatedLog log;
    log.type = LogType::STRIPE_MAP_UPDATED;
    log.vsid = 100;
    log.oldMap = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 0};
    log.newMap = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = 100};

    uint64_t bufferSize = sizeof(log);
    void* buffer = malloc(bufferSize);
    memcpy(buffer, &log, bufferSize);

    NiceMock<MockLogList> logList;

    // Then
    EXPECT_CALL(logList, AddLog(EqLog(buffer)));

    // When
    LogBufferParser parser;
    parser.GetLogs(buffer, bufferSize, logList);

    free(buffer);
}

TEST(LogBufferParser, GetLogs_testIfGcBlockWriteDoneLogIsParsed)
{
    // Given
    GcBlockWriteDoneLog log;
    log.type = LogType::GC_BLOCK_WRITE_DONE;
    log.volId = 1;
    log.vsid = 100;
    log.numBlockMaps = 2;

    GcBlockMapUpdate blockMapUpdates[log.numBlockMaps];
    blockMapUpdates[0].rba = 0;
    blockMapUpdates[0].vsa = {
        .stripeId = log.vsid,
        .offset = 0};
    blockMapUpdates[1].rba = 1;
    blockMapUpdates[1].vsa = {
        .stripeId = log.vsid,
        .offset = 1};

    uint64_t bufferSize = sizeof(log) + sizeof(blockMapUpdates);
    void* buffer = malloc(bufferSize);
    memcpy(buffer, &log, sizeof(log));
    memcpy((char*)buffer + sizeof(log), &blockMapUpdates, sizeof(blockMapUpdates));

    NiceMock<MockLogList> logList;

    // Then
    EXPECT_CALL(logList, AddLog(EqLog(buffer)));

    // When
    LogBufferParser parser;
    parser.GetLogs(buffer, bufferSize, logList);

    free(buffer);
}

TEST(LogBufferParser, GetLogs_testIfGcStripeFlushedLogIsParsed)
{
    // Given
    GcStripeFlushedLog log;
    log.type = LogType::GC_STRIPE_FLUSHED;
    log.volId = 1;
    log.vsid = 100;
    log.wbLsid = 20;
    log.userLsid = 100;
    log.totalNumBlockMaps = 2;

    uint64_t bufferSize = sizeof(log);
    void* buffer = malloc(bufferSize);
    memcpy(buffer, &log, sizeof(log));

    NiceMock<MockLogList> logList;

    // Then
    EXPECT_CALL(logList, AddLog(EqLog(buffer)));

    // When
    LogBufferParser parser;
    parser.GetLogs(buffer, bufferSize, logList);

    free(buffer);
}

TEST(LogBufferParser, GetLogs_testIfVolumeDeletedLogIsParsed)
{
    // Given
    VolumeDeletedLog log;
    log.type = LogType::VOLUME_DELETED;
    log.volId = 1;
    log.allocatorContextVersion = 10;

    uint64_t bufferSize = sizeof(log);
    void* buffer = malloc(bufferSize);
    memcpy(buffer, &log, sizeof(log));

    NiceMock<MockLogList> logList;

    // Then
    EXPECT_CALL(logList, AddLog(EqLog(buffer)));

    // When
    LogBufferParser parser;
    parser.GetLogs(buffer, bufferSize, logList);

    free(buffer);
}

TEST(LogBufferParser, GetLogs_testIfLogsAreParsed)
{
    // Given
    uint64_t logBufferSize = 16 * 1024;
    void* logBuffer = malloc(logBufferSize);

    NiceMock<MockLogList> logList;

    uint64_t currentOffset = 0;
    while (currentOffset + sizeof(BlockWriteDoneLog) < logBufferSize)
    {
        BlockWriteDoneLog log;
        log.type = LogType::BLOCK_WRITE_DONE;
        log.volId = 1;
        log.startRba = currentOffset;
        log.numBlks = 0;
        log.startVsa = {
            .stripeId = 0,
            .offset = 0};
        log.wbIndex = 0;
        log.writeBufferStripeAddress = {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = 0};

        char* targetBuffer = (char*)logBuffer + currentOffset;
        memcpy(targetBuffer, &log, sizeof(log));

        EXPECT_CALL(logList, AddLog(EqLog(targetBuffer)));

        currentOffset += sizeof(BlockWriteDoneLog);
    }

    // When
    LogBufferParser parser;
    parser.GetLogs(logBuffer, logBufferSize, logList);

    free(logBuffer);
}

TEST(LogBufferParser, GetLogs_testIfLogsAndLogBufferFooterAreParsed)
{
    // Given
    uint64_t logBufferSize = sizeof(BlockWriteDoneLog) * 10 + sizeof(LogGroupFooter);
    void* logBuffer = malloc(logBufferSize);

    NiceMock<MockLogList> logList;

    uint64_t currentOffset = 0;
    for (int count = 0; count < 10; count++)
    {
        BlockWriteDoneLog log;
        log.type = LogType::BLOCK_WRITE_DONE;
        log.volId = 1;
        log.startRba = currentOffset;
        log.numBlks = 0;
        log.startVsa = {
            .stripeId = 0,
            .offset = 0};
        log.wbIndex = 0;
        log.writeBufferStripeAddress = {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = 0};

        char* targetBuffer = (char*)logBuffer + currentOffset;
        memcpy(targetBuffer, &log, sizeof(log));

        EXPECT_CALL(logList, AddLog(EqLog(targetBuffer)));

        currentOffset += sizeof(BlockWriteDoneLog);
    }

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 13;
    footer.isReseted = false;

    char* targetBuffer = (char*)logBuffer + currentOffset;
    memcpy(targetBuffer, &footer, sizeof(LogGroupFooter));

    EXPECT_CALL(logList, SetLogGroupFooter(_, footer)).Times(1);

    // When
    LogBufferParser parser;
    parser.GetLogs(logBuffer, logBufferSize, logList);

    free(logBuffer);
}

TEST(LogBufferParser, GetLogs_testIfSeveralSequenceNumberSeen)
{
    // Given
    uint64_t logBufferSize = sizeof(BlockWriteDoneLog) * 10 + sizeof(LogGroupFooter);
    void* logBuffer = malloc(logBufferSize);

    NiceMock<MockLogList> logList;

    uint64_t currentOffset = 0;
    for (int count = 0; count < 5; count++)
    {
        BlockWriteDoneLog log;
        log.type = LogType::BLOCK_WRITE_DONE;
        log.seqNum = count;
        log.volId = 1;
        log.startRba = currentOffset;
        log.numBlks = 0;
        log.startVsa = {
            .stripeId = 0,
            .offset = 0};
        log.wbIndex = 0;
        log.writeBufferStripeAddress = {
            .stripeLoc = IN_WRITE_BUFFER_AREA,
            .stripeId = 0};

        char* targetBuffer = (char*)logBuffer + currentOffset;
        memcpy(targetBuffer, &log, sizeof(log));

        EXPECT_CALL(logList, AddLog(EqLog(targetBuffer)));

        currentOffset += sizeof(BlockWriteDoneLog);
    }

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 13;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 0;

    char* targetBuffer = (char*)logBuffer + currentOffset;
    memcpy(targetBuffer, &footer, sizeof(LogGroupFooter));

    // When
    LogBufferParser parser;
    int result = parser.GetLogs(logBuffer, logBufferSize, logList);

    // Then: LogBufferParser will return the error code
    int expect = static_cast<int>(EID(JOURNAL_INVALID_LOG_FOUND)) * -1;
    EXPECT_EQ(result, expect);
    free(logBuffer);
}
} // namespace pos
