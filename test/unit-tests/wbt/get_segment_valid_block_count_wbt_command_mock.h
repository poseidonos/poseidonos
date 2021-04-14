#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_segment_valid_block_count_wbt_command.h"

namespace pos
{
class MockGetSegmentValidBlockCountWbtCommand : public GetSegmentValidBlockCountWbtCommand
{
public:
    using GetSegmentValidBlockCountWbtCommand::GetSegmentValidBlockCountWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
