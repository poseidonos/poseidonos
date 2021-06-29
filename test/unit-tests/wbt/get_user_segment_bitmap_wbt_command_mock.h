#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_user_segment_bitmap_wbt_command.h"

namespace pos
{
class MockGetUserSegmentBitmapWbtCommand : public GetUserSegmentBitmapWbtCommand
{
public:
    using GetUserSegmentBitmapWbtCommand::GetUserSegmentBitmapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
