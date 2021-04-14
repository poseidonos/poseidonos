#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_user_segment_bitmap_wbt_command.h"

namespace pos
{
class MockSetUserSegmentBitmapWbtCommand : public SetUserSegmentBitmapWbtCommand
{
public:
    using SetUserSegmentBitmapWbtCommand::SetUserSegmentBitmapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
