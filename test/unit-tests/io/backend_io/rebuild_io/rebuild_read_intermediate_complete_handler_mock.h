#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/io/backend_io/rebuild_io/rebuild_read_intermediate_complete_handler.h"

namespace pos
{
class MockRebuildReadIntermediateCompleteHandler : public RebuildReadIntermediateCompleteHandler
{
public:
    using RebuildReadIntermediateCompleteHandler::RebuildReadIntermediateCompleteHandler;
    MOCK_METHOD(bool, _DoSpecificJob, (), (override));
};

} // namespace pos
