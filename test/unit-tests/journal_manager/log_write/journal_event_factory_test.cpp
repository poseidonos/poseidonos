#include "src/journal_manager/log_write/journal_event_factory.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/log_write/gc_log_write_completed_mock.h"
#include "test/unit-tests/journal_manager/log_write/gc_stripe_log_write_request_mock.h"
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

TEST(JournalEventFactory, CreateGcLogWriteCompletedEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);

    EventSmartPtr event = factory.CreateGcLogWriteCompletedEvent(nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(GcLogWriteCompleted));
}

TEST(JournalEventFactory, CreateGcStripeLogWriteRequestEvent_testIfCreatedSuccessfully)
{
    NiceMock<MockEventScheduler> eventScheduler;
    NiceMock<MockLogWriteHandler> logWriteHandler;

    JournalEventFactory factory;
    factory.Init(&eventScheduler, &logWriteHandler);

    EventSmartPtr event = factory.CreateGcStripeLogWriteRequestEvent(nullptr);
    EXPECT_EQ(typeid(*event.get()), typeid(GcStripeLogWriteRequest));
}
} // namespace pos
