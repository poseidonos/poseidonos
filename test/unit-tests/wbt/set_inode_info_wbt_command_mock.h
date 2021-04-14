#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/set_inode_info_wbt_command.h"

namespace pos
{
class MockSetInodeInfoWbtCommand : public SetInodeInfoWbtCommand
{
public:
    using SetInodeInfoWbtCommand::SetInodeInfoWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
