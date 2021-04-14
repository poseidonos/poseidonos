#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_segment_info_wbt_command.h"

namespace pos
{
class MockGetSegmentInfoWbtCommand : public GetSegmentInfoWbtCommand
{
public:
    using GetSegmentInfoWbtCommand::GetSegmentInfoWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
