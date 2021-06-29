#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/i_log_group_reset_completed.h"

namespace pos
{
class MockILogGroupResetCompleted : public ILogGroupResetCompleted
{
public:
    using ILogGroupResetCompleted::ILogGroupResetCompleted;
    MOCK_METHOD(void, LogGroupResetCompleted, (int logGroupId), (override));
};

} // namespace pos
