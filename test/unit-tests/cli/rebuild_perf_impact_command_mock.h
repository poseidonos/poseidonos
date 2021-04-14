#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/cli/rebuild_perf_impact_command.h"

namespace pos_cli
{
class MockRebuildPerfImpactCommand : public RebuildPerfImpactCommand
{
public:
    using RebuildPerfImpactCommand::RebuildPerfImpactCommand;
    MOCK_METHOD(string, Execute, (json & doc, string rid), (override));
};

} // namespace pos_cli
