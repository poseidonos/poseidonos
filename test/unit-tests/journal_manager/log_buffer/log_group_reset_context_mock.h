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
    MOCK_METHOD(void, SetCallback, (MetaFileIoCbPtr callback), (override));
    MOCK_METHOD(void, SetIoRequest, (uint64_t offset, uint64_t len, char* buf), (override));
    MOCK_METHOD(void, SetFileInfo, (int fd, MetaFileIoDoneCheckFunc ioDoneCheckCallback), (override));
    MOCK_METHOD(void, IoDone, (), (override));

};

} // namespace pos
