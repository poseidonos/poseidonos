#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_wb_lsid_bitmap_wbt_command.h"

namespace pos
{
class MockSetWbLsidBitmapWbtCommand : public SetWbLsidBitmapWbtCommand
{
public:
    using SetWbLsidBitmapWbtCommand::SetWbLsidBitmapWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
