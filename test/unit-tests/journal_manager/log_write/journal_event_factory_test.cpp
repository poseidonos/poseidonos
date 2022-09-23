#include "src/journal_manager/log_write/journal_event_factory.h"

#include <gtest/gtest.h>

#include "src/journal_manager/log_write/gc_log_write.h"
#include "src/journal_manager/log_write/gc_log_write_completed.h"
#include "src/journal_manager/log_write/log_write_request.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(JournalEventFactory, Init_testIfExecutedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);
}

TEST(JournalEventFactory, CreateGcBlockLogWriteCompletedEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);

    EventSmartPtr event = factory.CreateGcBlockLogWriteCompletedEvent(nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(GcLogWriteCompleted));
}

TEST(JournalEventFactory, CreateLogWriteEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);

    EventSmartPtr event = factory.CreateLogWriteEvent(nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(LogWriteRequest));
}

TEST(JournalEventFactory, CreateGcLogWriteEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);

    std::vector<LogWriteContext*> emptyList;
    EventSmartPtr event = factory.CreateGcLogWriteEvent(emptyList, nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(GcLogWrite));
}
} // namespace pos
