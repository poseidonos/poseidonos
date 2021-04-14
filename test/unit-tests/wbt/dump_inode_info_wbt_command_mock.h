#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/dump_inode_info_wbt_command.h"

namespace pos
{
class MockDumpInodeInfoWbtCommand : public DumpInodeInfoWbtCommand
{
public:
    using DumpInodeInfoWbtCommand::DumpInodeInfoWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
