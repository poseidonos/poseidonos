#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_segment_info_wbt_command.h"

namespace pos
{
class MockSetSegmentInfoWbtCommand : public SetSegmentInfoWbtCommand
{
public:
    using SetSegmentInfoWbtCommand::SetSegmentInfoWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
