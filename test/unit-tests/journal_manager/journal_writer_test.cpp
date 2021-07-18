#include "src/journal_manager/journal_writer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/journaling_status_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(JournalWriter, Init_testIfInitializedSuccessfully)
{
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillOnce(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    MpageList dummyList;
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, dummyList, nullptr) == 0);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    MpageList dummyList;
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, dummyList, nullptr) < 0);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    MpageList dummyList;
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, dummyList, nullptr) > 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillOnce(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    MpageList dummyList;
    StripeAddr unmap = {.stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, dummyList, nullptr) == 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    MpageList dummyList;
    StripeAddr unmap = {.stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, dummyList, nullptr) < 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    MpageList dummyList;
    StripeAddr unmap = {.stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, dummyList, nullptr) > 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillOnce(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    GcStripeMapUpdateList dummyMapUpdates;
    MapPageList dummyList;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(dummyMapUpdates, dummyList, nullptr) == 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    GcStripeMapUpdateList dummyMapUpdates;
    MapPageList dummyList;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(dummyMapUpdates, dummyList, nullptr) < 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    GcStripeMapUpdateList dummyMapUpdates;
    MapPageList dummyList;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(dummyMapUpdates, dummyList, nullptr) > 0);
}

} // namespace pos
