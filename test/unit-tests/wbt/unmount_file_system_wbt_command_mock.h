#include <gmock/gmock.h>

#include <list>
#include <string>
#include <vector>

#include "src/wbt/unmount_file_system_wbt_command.h"

namespace pos
{
class MockUnmountFileSystemWbtCommand : public UnmountFileSystemWbtCommand
{
public:
    using UnmountFileSystemWbtCommand::UnmountFileSystemWbtCommand;
    MOCK_METHOD(int, Execute, (Args & argv, JsonElement& elem), (override));
};

} // namespace pos
