#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_bitmap_layout_wbt_command.h"

namespace pos
{
class MockGetBitmapLayoutWbtCommand : public GetBitmapLayoutWbtCommand
{
public:
    using GetBitmapLayoutWbtCommand::GetBitmapLayoutWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
