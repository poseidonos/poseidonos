#include "src/journal_manager/log_write/gc_log_write_completed.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using ::testing::NiceMock;

namespace pos
{

TEST(GcLogWriteCompleted, Execute_testWhenNumLogsIsOne)
{
    NiceMock<MockEventScheduler> eventScheduler;
    EventSmartPtr event(new MockEvent());

    GcLogWriteCompleted completed(&eventScheduler, event);
    completed.SetNumLogs(1);

    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    completed.Execute();
}

TEST(GcLogWriteCompleted, Execute_testWhenNumLogsIsBiggerThanOne)
{
    NiceMock<MockEventScheduler> eventScheduler;
    EventSmartPtr event(new MockEvent());

    GcLogWriteCompleted completed(&eventScheduler, event);

    int numLogs = 10;
    completed.SetNumLogs(10);

    EXPECT_CALL(eventScheduler, EnqueueEvent).Times(1);

    for (int count = 0; count < numLogs; count++)
    {
        completed.Execute();
    }
}
} // namespace pos
