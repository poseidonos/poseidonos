/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/journal_manager/journal_writer.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/journaling_status_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/gc_log_write_completed_mock.h"
#include "test/unit-tests/journal_manager/log_write/journal_event_factory_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(JournalWriter, Init_testIfInitializedSuccessfully)
{
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfExectedWithoutInitialilzation)
{
    // Given
    JournalWriter writer;

    // When: Journal is requested to write block write done log without initialization
    int actualReturnCode = writer.AddBlockMapUpdatedLog(nullptr, nullptr);

    // Then: JournalWriter return the error code;
    int expectedReturnCode = ERRID(JOURNAL_INVALID);
    EXPECT_EQ(expectedReturnCode, actualReturnCode);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillOnce(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, nullptr) == 0);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, nullptr) < 0);
}

TEST(JournalWriter, AddBlockMapUpdatedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    EXPECT_TRUE(writer.AddBlockMapUpdatedLog(nullptr, nullptr) > 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillOnce(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    StripeAddr unmap = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, nullptr) == 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    StripeAddr unmap = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, nullptr) < 0);
}

TEST(JournalWriter, AddStripeMapUpdatedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    StripeAddr unmap = {.stripeLoc = StripeLoc::IN_WRITE_BUFFER_AREA, .stripeId = UNMAP_STRIPE};
    EXPECT_TRUE(writer.AddStripeMapUpdatedLog(nullptr, unmap, nullptr) > 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfSuccessWhenStatusIsJournaling)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    EventSmartPtr callbackEvent(new MockGcLogWriteCompleted());
    ON_CALL(eventFactory, CreateGcLogWriteCompletedEvent).WillByDefault(Return(callbackEvent));

    auto eventPtr = dynamic_cast<MockGcLogWriteCompleted*>(callbackEvent.get());
    assert(eventPtr != nullptr);
    EXPECT_CALL(*eventPtr, Execute).Times(0);

    // Then: Should request writing logs to write handler
    EXPECT_CALL(writeHandler, AddLog).WillRepeatedly(Return(0));

    // When: Journal is requested to write block write done log
    // Then: Log write should be successfully requested
    GcStripeMapUpdateList mapUpdates;
    GcBlockMapUpdate dummyUpdate = {
        .rba = 0,
        .vsa = UNMAP_VSA};
    mapUpdates.blockMapUpdateList.assign(5, dummyUpdate);
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(mapUpdates, nullptr) == 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfFailsWhenStatusIsInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNAL_INVALID));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Log write should be failed
    GcStripeMapUpdateList dummyMapUpdates;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(dummyMapUpdates, nullptr) < 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfFailsWhenStatusIsNotJournalingOrInvalid)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(REPLAYING_JOURNAL));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    GcStripeMapUpdateList dummyMapUpdates;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(dummyMapUpdates, nullptr) > 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfGcLogWriteCompletionCallbackIsExecutedWhenBlockMapIsEmpty)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    EventSmartPtr callbackEvent(new MockGcLogWriteCompleted());
    ON_CALL(eventFactory, CreateGcLogWriteCompletedEvent).WillByDefault(Return(callbackEvent));

    auto eventPtr = dynamic_cast<MockGcLogWriteCompleted*>(callbackEvent.get());
    assert(eventPtr != nullptr);
    EXPECT_CALL(*eventPtr, SetNumLogs);
    EXPECT_CALL(*eventPtr, Execute);

    // When: Requested to write block write done log
    // Then: Should return value above zero to let caller retry it
    GcStripeMapUpdateList emptyMapUpdates;
    EXPECT_TRUE(writer.AddGcStripeFlushedLog(emptyMapUpdates, nullptr) == 0);
}

TEST(JournalWriter, AddGcStripeFlushedLog_testIfFailedToAddLog)
{
    // Given
    NiceMock<MockLogWriteHandler> writeHandler;
    NiceMock<MockLogWriteContextFactory> factory;
    NiceMock<MockJournalEventFactory> eventFactory;
    NiceMock<MockJournalingStatus> status;

    ON_CALL(status, Get).WillByDefault(Return(JOURNALING));

    JournalWriter writer;
    writer.Init(&writeHandler, &factory, &eventFactory, &status);

    EventSmartPtr callbackEvent(new MockGcLogWriteCompleted());
    ON_CALL(eventFactory, CreateGcLogWriteCompletedEvent).WillByDefault(Return(callbackEvent));

    // When
    GcStripeMapUpdateList mapUpdates;
    GcBlockMapUpdate dummyUpdate = {
        .rba = 0,
        .vsa = UNMAP_VSA};
    mapUpdates.blockMapUpdateList.assign(5, dummyUpdate);

    std::vector<LogWriteContext*> blockContexts;
    MapUpdateLogWriteContext logWriteContext;
    blockContexts.push_back(&logWriteContext);
    int numDummyContexts = 4;
    for (int index = 0; index < numDummyContexts; index++)
    {
        MapUpdateLogWriteContext* logWriteContext = new MapUpdateLogWriteContext;
        blockContexts.push_back(logWriteContext);
    }
    EXPECT_CALL(factory, CreateGcBlockMapLogWriteContexts).WillOnce(Return(blockContexts));
    EXPECT_CALL(writeHandler, AddLog).WillRepeatedly(Return(-1));

    EXPECT_TRUE(writer.AddGcStripeFlushedLog(mapUpdates, nullptr) == -1);
}
} // namespace pos
