#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"

namespace pos
{
class MockLogGroupFooterWriteContext : public LogGroupFooterWriteContext
{
public:
    using LogGroupFooterWriteContext::LogGroupFooterWriteContext;
};

} // namespace pos
