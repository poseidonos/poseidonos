#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"

namespace pos
{
class MockLogGroupResetCompletedEvent : public LogGroupResetCompletedEvent
{
public:
    using LogGroupResetCompletedEvent::LogGroupResetCompletedEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
