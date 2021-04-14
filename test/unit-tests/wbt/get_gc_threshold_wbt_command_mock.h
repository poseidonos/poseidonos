#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_gc_threshold_wbt_command.h"

namespace pos
{
class MockGetGcThresholdWbtCommand : public GetGcThresholdWbtCommand
{
public:
    using GetGcThresholdWbtCommand::GetGcThresholdWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
