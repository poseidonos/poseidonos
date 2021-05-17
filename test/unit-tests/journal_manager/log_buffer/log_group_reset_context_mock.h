#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/log_group_reset_context.h"

namespace pos
{
class MockLogGroupResetContext : public LogGroupResetContext
{
public:
    using LogGroupResetContext::LogGroupResetContext;
};

} // namespace pos
