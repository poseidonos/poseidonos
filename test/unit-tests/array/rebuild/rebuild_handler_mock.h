#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/array/rebuild/rebuild_handler.h"

namespace pos
{
class MockRebuildHandler : public RebuildHandler
{
public:
    using RebuildHandler::RebuildHandler;
    MOCK_METHOD(bool, Execute, (), (override));
};

} // namespace pos
