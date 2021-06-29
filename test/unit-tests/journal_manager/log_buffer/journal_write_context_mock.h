#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/journal_write_context.h"

namespace pos
{
class MockJournalResetContext : public LogGroupResetContext
{
public:
    using LogGroupResetContext::LogGroupResetContext;
};

} // namespace pos
