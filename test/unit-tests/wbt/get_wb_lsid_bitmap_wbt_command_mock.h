#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/get_wb_lsid_bitmap_wbt_command.h"

namespace pos
{
class MockGetWbLsidBitmapWbtCommand : public GetWbLsidBitmapWbtCommand
{
public:
    using GetWbLsidBitmapWbtCommand::GetWbLsidBitmapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
