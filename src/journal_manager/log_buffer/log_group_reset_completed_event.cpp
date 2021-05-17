#include "src/journal_manager/log_buffer/log_group_reset_completed_event.h"

namespace pos
{
LogGroupResetCompletedEvent::LogGroupResetCompletedEvent(ILogGroupResetCompleted* target, int id)
: target(target),
  logGroupId(id)
{
}

bool
LogGroupResetCompletedEvent::Execute(void)
{
    target->LogGroupResetCompleted(logGroupId);

    return true;
}
} // namespace pos
