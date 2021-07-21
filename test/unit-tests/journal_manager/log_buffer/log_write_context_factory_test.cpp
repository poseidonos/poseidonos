#include "src/journal_manager/log_buffer/log_write_context_factory.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
const uint64_t MAX_LOG_SIZE = 4032;

TEST(LogWriteContextFactory, LogWriteContextFactory_)
{
}

TEST(LogWriteContextFactory, Init_)
{
}

TEST(LogWriteContextFactory, CreateBlockMapLogWriteContext_)
{
}

TEST(LogWriteContextFactory, CreateStripeMapLogWriteContext_)
{
}

TEST(LogWriteContextFactory, CreateGcBlockMapLogWriteContext_)
{
}

TEST(LogWriteContextFactory, CreateGcBlockMapLogWriteContexts_testCreatingSmallLogs)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetMetaPageSize).WillByDefault(Return(MAX_LOG_SIZE));

    LogWriteContextFactory factory;
    factory.Init(&config, nullptr, nullptr);

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
    MapPageList dummy;
    auto createdContexts = factory.CreateGcBlockMapLogWriteContexts(mapUpdates, dummy, nullptr);

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
    factory.Init(&config, nullptr, nullptr);

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
    MapPageList dummy;
    auto createdContexts = factory.CreateGcBlockMapLogWriteContexts(mapUpdates, dummy, nullptr);

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

TEST(LogWriteContextFactory, CreateGcStripeFlushedLogWriteContext_)
{
}

TEST(LogWriteContextFactory, CreateVolumeDeletedLogWriteContext_)
{
}

} // namespace pos
