#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/log_group_footer_write_event.h"

namespace pos
{
class MockLogGroupFooterWriteEvent : public LogGroupFooterWriteEvent
{
public:
    using LogGroupFooterWriteEvent::LogGroupFooterWriteEvent;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
