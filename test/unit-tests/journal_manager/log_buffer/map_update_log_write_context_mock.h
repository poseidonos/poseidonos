#include <gmock/gmock.h>
#include <string>
#include <list>
#include <vector>
#include "src/journal_manager/log_buffer/map_update_log_write_context.h"

namespace pos
{
class MockMapUpdateLogWriteContext : public MapUpdateLogWriteContext
{
public:
    using MapUpdateLogWriteContext::MapUpdateLogWriteContext;
    MOCK_METHOD(void, IoDone, (), (override));
};

} // namespace pos
