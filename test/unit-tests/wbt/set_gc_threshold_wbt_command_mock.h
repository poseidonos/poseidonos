#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_gc_threshold_wbt_command.h"

namespace pos
{
class MockSetGcThresholdWbtCommand : public SetGcThresholdWbtCommand
{
public:
    using SetGcThresholdWbtCommand::SetGcThresholdWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
