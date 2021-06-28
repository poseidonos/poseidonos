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
    MOCK_METHOD(void, SetInternalCallback, (MetaIoCbPtr cb), (override));
    MOCK_METHOD(void, SetFile, (int fileDescriptor), (override));
    MOCK_METHOD(void, IoDone, (), (override));
};

} // namespace pos
