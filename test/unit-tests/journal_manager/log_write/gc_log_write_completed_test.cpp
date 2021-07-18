#include "src/journal_manager/log_write/gc_log_write_completed.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"

using ::testing::NiceMock;

namespace pos
{
int
TestCallback(LogWriteContext* context)
{
    context->IoDone();

    delete context;
    return 0;
}

TEST(GcLogWriteCompleted, Execute_testWhenNumLogsIsOne)
{
    GcLogWriteCallback callbackFunc = std::bind(&TestCallback, std::placeholders::_1);
    NiceMock<MockLogWriteContext>* logWriteContext = new NiceMock<MockLogWriteContext>;

    GcLogWriteCompleted completed(callbackFunc, logWriteContext);
    completed.SetNumLogs(1);

    EXPECT_CALL(*logWriteContext, IoDone).Times(1);

    completed.Execute();
}

TEST(GcLogWriteCompleted, Execute_testWhenNumLogsIsBiggerThanOne)
{
    GcLogWriteCallback callbackFunc = std::bind(&TestCallback, std::placeholders::_1);
    NiceMock<MockLogWriteContext>* logWriteContext = new NiceMock<MockLogWriteContext>;

    GcLogWriteCompleted completed(callbackFunc, logWriteContext);

    int numLogs = 10;
    completed.SetNumLogs(10);

    EXPECT_CALL(*logWriteContext, IoDone).Times(1);

    for (int count = 0; count < numLogs; count++)
    {
        completed.Execute();
    }
}
} // namespace pos
