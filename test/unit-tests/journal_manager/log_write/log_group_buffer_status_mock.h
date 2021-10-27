#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/journal_manager/log_write/log_group_buffer_status.h"

namespace pos
{
class MockLogGroupBufferStatus : public LogGroupBufferStatus
{
public:
    using LogGroupBufferStatus::LogGroupBufferStatus;
    MOCK_METHOD(bool, TryToSetFull, (), (override));
    MOCK_METHOD(void, LogFilled, (), (override));
};

} // namespace pos
