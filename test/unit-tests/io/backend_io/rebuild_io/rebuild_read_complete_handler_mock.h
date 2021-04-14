#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/rebuild_io/rebuild_read_complete_handler.h"

namespace pos
{
class MockRebuildReadCompleteHandler : public RebuildReadCompleteHandler
{
public:
    using RebuildReadCompleteHandler::RebuildReadCompleteHandler;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
