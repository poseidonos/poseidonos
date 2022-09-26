#include "src/journal_manager/log_write/log_write_request.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/log_buffer/log_write_context_mock.h"
#include "test/unit-tests/journal_manager/log_write/log_write_handler_mock.h"

namespace pos
{
using ::testing::NiceMock;
using ::testing::Return;

TEST(LogWriteRequest, DoSpecificJob_testIfLogWriteCompletedWhenSuccess)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;
    NiceMock<MockLogWriteContext> logWriteContext;

    LogWriteRequest request(&logWriteHandler, &logWriteContext);

    EXPECT_CALL(logWriteHandler, AddLog).WillOnce(Return(0));

    EXPECT_EQ(request.Execute(), true);
}

TEST(LogWriteRequest, DoSpecificJob_testIfLogWriteRetriedWhenFailed)
{
    NiceMock<MockLogWriteHandler> logWriteHandler;
    NiceMock<MockLogWriteContext> logWriteContext;

    LogWriteRequest request(&logWriteHandler, &logWriteContext);

    EXPECT_CALL(logWriteHandler, AddLog).WillOnce(Return(-1));

    EXPECT_EQ(request.Execute(), false);
}

} // namespace pos
