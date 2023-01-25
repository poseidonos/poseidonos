#include "src/journal_manager/log_buffer/log_buffer_io_context_factory.h"

#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(LogBufferIoContextFactory, Init_testIfExecutedSuccessfully)
{
    LogBufferIoContextFactory factory;

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    factory.Init(&config, &notifier, &sequencer);
}

TEST(LogBufferIoContextFactory, CreateLogBufferIoContext_testCreation)
{
    LogBufferIoContextFactory factory;

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    factory.Init(&config, &notifier, &sequencer);

    auto result = factory.CreateLogBufferIoContext(0, nullptr);

    EXPECT_EQ(typeid(*result), typeid(LogBufferIoContext));
}

TEST(LogBufferIoContextFactory, CreateMapUpdateLogWriteIoContext_testCreation)
{
    LogBufferIoContextFactory factory;

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    factory.Init(&config, &notifier, &sequencer);

    NiceMock<MockLogWriteContext> context;
    auto result = factory.CreateMapUpdateLogWriteIoContext(&context);

    EXPECT_EQ(typeid(*result), typeid(MapUpdateLogWriteContext));
}

TEST(LogBufferIoContextFactory, CreateLogWriteIoContext_testCreation)
{
    LogBufferIoContextFactory factory;

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    factory.Init(&config, &notifier, &sequencer);

    NiceMock<MockLogWriteContext> context;
    auto result = factory.CreateLogWriteIoContext(&context);

    EXPECT_EQ(typeid(*result), typeid(LogWriteIoContext));
}

TEST(LogBufferIoContextFactory, CreateLogGroupFooterWriteContext_testCreation)
{
    LogBufferIoContextFactory factory;

    NiceMock<MockJournalConfiguration> config;
    NiceMock<MockLogBufferWriteDoneNotifier> notifier;
    NiceMock<MockCallbackSequenceController> sequencer;

    factory.Init(&config, &notifier, &sequencer);

    LogGroupFooter footer;
    footer.lastCheckpointedSeginfoVersion = 3;
    footer.isReseted = false;
    footer.resetedSequenceNumber = 4;

    auto result = factory.CreateLogGroupFooterWriteContext(0, footer, 0, nullptr);

    EXPECT_EQ(typeid(*result), typeid(LogGroupFooterWriteContext));
    EXPECT_EQ(*(LogGroupFooter*)dynamic_cast<LogGroupFooterWriteContext*>(result)->GetBuffer(), footer);
}

} // namespace pos
