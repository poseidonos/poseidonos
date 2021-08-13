#include "src/journal_manager/log_write/journal_event_factory.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(JournalEventFactory, Init_testIfExecutedSuccessfully)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&logWriteHandler);
}

TEST(JournalEventFactory, CreateGcLogWriteCompletedEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&logWriteHandler);

    EventSmartPtr event = factory.CreateGcLogWriteCompletedEvent(nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(GcLogWriteCompleted));
}

} // namespace pos
