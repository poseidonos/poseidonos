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
    MOCK_METHOD(void, SetCallback, (MetaIoCbPtr callback), (override));
    MOCK_METHOD(void, SetIoRequest, (uint64_t offset, uint64_t len, char* buf), (override));
    MOCK_METHOD(void, SetFileInfo, (int fd, MetaFileIoCbPtr ioDoneCheckCallback), (override));
    MOCK_METHOD(void, IoDone, (), (override));

};

} // namespace pos
