#include "src/journal_manager/log_buffer/log_write_context_factory.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/memory.h"
#include "src/journal_manager/log/block_write_done_log_handler.h"
#include "src/journal_manager/log/gc_stripe_flushed_log_handler.h"
#include "src/journal_manager/log/stripe_map_updated_log_handler.h"
#include "src/journal_manager/log/volume_deleted_log_handler.h"
#include "src/journal_manager/log_buffer/log_group_reset_context.h"
#include "test/unit-tests/allocator/stripe/stripe_mock.h"
#include "test/unit-tests/bio/volume_io_mock.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"

using testing::NiceMock;
using testing::Return;
using testing::ReturnRef;

const uint64_t MAX_LOG_SIZE = 4032;

namespace pos
{
TEST(LogWriteContextFactory, Init_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;

    // When
    logWriteContextFactory.Init(&config, &notifier);

    // Then
    EXPECT_EQ(&notifier, logWriteContextFactory.GetLogBufferWriteDoneNotifier());
}

TEST(LogWriteContextFactory, CreateBlockMapLogWriteContext_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;
    logWriteContextFactory.Init(&config, &notifier);

    // When
    EventSmartPtr callbackEvent;
    NiceMock<MockVolumeIo>* volumeIo = new NiceMock<MockVolumeIo>;
    uint32_t volumeId = 1;
    uint64_t sectorRba = 100;
    uint64_t blockSize = 1024;
    VirtualBlkAddr startVsa = {
        .stripeId = 0,
        .offset = 10};
    StripeAddr wbAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = 100};
    EXPECT_CALL(*volumeIo, GetVolumeId).WillRepeatedly(Return(volumeId));
    EXPECT_CALL(*volumeIo, GetSectorRba).WillOnce(Return(sectorRba));
    EXPECT_CALL(*volumeIo, GetSize).WillOnce(Return(blockSize));
    EXPECT_CALL(*volumeIo, GetVsa).WillOnce(ReturnRef(startVsa));
    EXPECT_CALL(*volumeIo, GetLsidEntry).WillOnce(ReturnRef(wbAddr));

    LogWriteContext* logWriteContext = logWriteContextFactory.CreateBlockMapLogWriteContext(VolumeIoSmartPtr(volumeIo), callbackEvent);

    // Then
    uint64_t startRba = ChangeSectorToBlock(sectorRba);
    uint64_t numBlks = DivideUp(blockSize, BLOCK_SIZE);
    BlockWriteDoneLogHandler expectLog(volumeId, startRba, numBlks, startVsa, volumeId, wbAddr);
    BlockWriteDoneLogHandler* actualLog = dynamic_cast<BlockWriteDoneLogHandler*>(logWriteContext->GetLog());
    EXPECT_TRUE(actualLog != nullptr);
    EXPECT_EQ(expectLog, *actualLog);

    MapList expectDirtyMap;
    expectDirtyMap.emplace(volumeId);

    EXPECT_EQ(expectDirtyMap, dynamic_cast<MapUpdateLogWriteContext*>(logWriteContext)->GetDirtyList());

    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(&notifier, dynamic_cast<LogWriteContext*>(logWriteContext)->GetLogBufferWriteDoneNotifier());
}

TEST(LogWriteContextFactory, CreateStripeMapLogWriteContext_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;
    logWriteContextFactory.Init(&config, &notifier);

    // When
    StripeId vsid = 100;
    StripeId wbLsid = 1;
    StripeAddr oldAddr = {
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = wbLsid};
    NiceMock<MockStripe> stripe;
    EXPECT_CALL(stripe, GetUserLsid).WillOnce(Return(vsid));
    EXPECT_CALL(stripe, GetVsid).WillOnce(Return(vsid));
    EventSmartPtr callbackEvent;
    LogWriteContext* logWriteContext = logWriteContextFactory.CreateStripeMapLogWriteContext(&stripe, oldAddr, callbackEvent);

    // Then
    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = vsid};
    StripeMapUpdatedLogHandler expectLog(vsid, oldAddr, newAddr);
    StripeMapUpdatedLogHandler* actualLog = dynamic_cast<StripeMapUpdatedLogHandler*>(logWriteContext->GetLog());
    EXPECT_TRUE(actualLog != nullptr);
    EXPECT_EQ(expectLog, *actualLog);

    MapList expectDirtyMap;
    expectDirtyMap.emplace(STRIPE_MAP_ID);
    EXPECT_EQ(expectDirtyMap, dynamic_cast<MapUpdateLogWriteContext*>(logWriteContext)->GetDirtyList());

    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(&notifier, dynamic_cast<LogWriteContext*>(logWriteContext)->GetLogBufferWriteDoneNotifier());
}

TEST(LogWriteContextFactory, CreateGcBlockMapLogWriteContexts_testCreatingSmallLogs)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetMetaPageSize).WillByDefault(Return(MAX_LOG_SIZE));

    LogWriteContextFactory factory;
    factory.Init(&config, nullptr);

    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = 10;
    mapUpdates.vsid = 1023;
    mapUpdates.wbLsid = 0;
    mapUpdates.userLsid = 1023;

    uint64_t numBlocks = 1;
    for (uint64_t index = 0; index < numBlocks; index++)
    {
        GcBlockMapUpdate update = {
            .rba = 1023 + index,
            .vsa = {
                .stripeId = 1023,
                .offset = index}};
        mapUpdates.blockMapUpdateList.push_back(update);
    }

    // When
    auto createdContexts = factory.CreateGcBlockMapLogWriteContexts(mapUpdates, nullptr);

    // Then
    EXPECT_EQ(createdContexts.size(), 1);
    EXPECT_TRUE(createdContexts.front()->GetLength() < MAX_LOG_SIZE);

    GcBlockWriteDoneLog log = *reinterpret_cast<GcBlockWriteDoneLog*>(createdContexts.front()->buffer);
    EXPECT_EQ(log.volId, mapUpdates.volumeId);
    EXPECT_EQ(log.vsid, mapUpdates.vsid);
    EXPECT_EQ(log.numBlockMaps, numBlocks);

    for (uint64_t index = 0; index < numBlocks; index++)
    {
        GcBlockMapUpdate expected = {
            .rba = 1023 + index,
            .vsa = {
                .stripeId = 1023,
                .offset = index}};
        GcBlockMapUpdate actual = *reinterpret_cast<GcBlockMapUpdate*>
            (createdContexts.front()->buffer + sizeof(GcBlockWriteDoneLog) + sizeof(GcBlockMapUpdate) * index);

        EXPECT_EQ(expected.rba, actual.rba);
        EXPECT_EQ(expected.vsa, actual.vsa);
    }
}

TEST(LogWriteContextFactory, CreateGcBlockMapLogWriteContexts_testIfLogsAreSpliitedWhenRequestedSizeIsLarge)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetMetaPageSize).WillByDefault(Return(MAX_LOG_SIZE));

    LogWriteContextFactory factory;
    factory.Init(&config, nullptr);

    GcStripeMapUpdateList mapUpdates;
    mapUpdates.volumeId = 10;
    mapUpdates.vsid = 1023;
    mapUpdates.wbLsid = 0;
    mapUpdates.userLsid = 1023;

    uint64_t numBlocks = 3685;
    for (uint64_t index = 0; index < numBlocks; index++)
    {
        GcBlockMapUpdate update = {
            .rba = 1023 + index,
            .vsa = {
                .stripeId = 1023,
                .offset = index}};
        mapUpdates.blockMapUpdateList.push_back(update);
    }

    // When
    auto createdContexts = factory.CreateGcBlockMapLogWriteContexts(mapUpdates, nullptr);

    // Then
    uint64_t numBlockMaps = 0;
    for (auto context : createdContexts)
    {
        EXPECT_TRUE(context->GetLength() < MAX_LOG_SIZE);

        GcBlockWriteDoneLog log = *reinterpret_cast<GcBlockWriteDoneLog*>(context->buffer);
        EXPECT_EQ(log.volId, mapUpdates.volumeId);
        EXPECT_EQ(log.vsid, mapUpdates.vsid);

        for (uint64_t index = 0; index < log.numBlockMaps; index++)
        {
            GcBlockMapUpdate expected = {
                .rba = 1023 + numBlockMaps + index,
                .vsa = {
                    .stripeId = 1023,
                    .offset = numBlockMaps + index}};
            GcBlockMapUpdate actual = *reinterpret_cast<GcBlockMapUpdate*>
                (context->buffer + sizeof(GcBlockWriteDoneLog) + sizeof(GcBlockMapUpdate) * index);

            EXPECT_EQ(expected.rba, actual.rba);
            EXPECT_EQ(expected.vsa, actual.vsa);
        }
        numBlockMaps += log.numBlockMaps;
    }

    EXPECT_EQ(numBlockMaps, numBlocks);
}

TEST(LogWriteContextFactory, CreateGcStripeFlushedLogWriteContext_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;
    logWriteContextFactory.Init(&config, &notifier);

    // When
    int volumeId = 1;
    GcStripeMapUpdateList mapUpdates = {
        .volumeId = volumeId,
        .blockMapUpdateList = std::vector<GcBlockMapUpdate>() /* not interesting */,
        .vsid = 100,
        .wbLsid = 1,
        .userLsid = 100};
    EventSmartPtr callbackEvent;
    LogWriteContext* logWriteContext = logWriteContextFactory.CreateGcStripeFlushedLogWriteContext(mapUpdates, callbackEvent);

    // Then
    GcStripeFlushedLogHandler expectLog(mapUpdates.volumeId, mapUpdates.vsid, mapUpdates.wbLsid,
        mapUpdates.userLsid, mapUpdates.blockMapUpdateList.size());
    GcStripeFlushedLogHandler* actualLog = dynamic_cast<GcStripeFlushedLogHandler*>(logWriteContext->GetLog());
    EXPECT_TRUE(actualLog != nullptr);
    EXPECT_EQ(expectLog, *actualLog);

    MapList expectDirtyMap;
    expectDirtyMap.emplace(volumeId);
    expectDirtyMap.emplace(STRIPE_MAP_ID);
    EXPECT_EQ(expectDirtyMap, dynamic_cast<MapUpdateLogWriteContext*>(logWriteContext)->GetDirtyList());
    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(&notifier, dynamic_cast<LogWriteContext*>(logWriteContext)->GetLogBufferWriteDoneNotifier());
}

TEST(LogWriteContextFactory, CreateVolumeDeletedLogWriteContext_testIfExecutedSuccessfully)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    LogWriteContextFactory logWriteContextFactory;
    logWriteContextFactory.Init(&config, &notifier);

    // When
    int volumeId = 1;
    uint64_t contextVersion = 10;
    EventSmartPtr callbackEvent;
    LogWriteContext* logWriteContext = logWriteContextFactory.CreateVolumeDeletedLogWriteContext(volumeId, contextVersion, callbackEvent);

    // Then
    VolumeDeletedLogEntry expectLog(volumeId, contextVersion);
    VolumeDeletedLogEntry* actualLog = dynamic_cast<VolumeDeletedLogEntry*>(logWriteContext->GetLog());
    EXPECT_TRUE(actualLog != nullptr);
    EXPECT_EQ(expectLog, *actualLog);

    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(&notifier, dynamic_cast<LogWriteContext*>(logWriteContext)->GetLogBufferWriteDoneNotifier());
}

TEST(LogWriteContextFactory, CreateLogGroupResetContext_testIfExecutedSuccessfully)
{
    // Given
    LogWriteContextFactory logWriteContextFactory;

    // When
    uint64_t offset = 0;
    int logGroupId = 1;
    uint64_t groupSize = 512;
    EventSmartPtr callbackEvent;
    char* initializedDataBuffer = new char[groupSize];
    memset(initializedDataBuffer, 0xFF, groupSize);
    LogGroupResetContext* logWriteContext = logWriteContextFactory.CreateLogGroupResetContext(offset, logGroupId, groupSize, callbackEvent, initializedDataBuffer);

    // Then
    EXPECT_EQ(logGroupId, logWriteContext->GetLogGroupId());
    EXPECT_EQ(callbackEvent, dynamic_cast<LogBufferIoContext*>(logWriteContext)->GetClientCallback());
    EXPECT_EQ(offset, logWriteContext->fileOffset);
    EXPECT_EQ(groupSize, logWriteContext->GetLength());
    EXPECT_EQ(initializedDataBuffer, logWriteContext->buffer);
}
} // namespace pos
